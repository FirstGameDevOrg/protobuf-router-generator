#include "ProtobufRouterGenerator.h"
#include <QFile>
#include <QFileDialog>
#include <QRegExp>
#include <QPushButton>
#include <QClipboard>
#include <QTimer>
#include <QCheckBox>
#include <QMessageBox>
#include "MessageWrapper.hpp"

ProtobufRouterGenerator::ProtobufRouterGenerator(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.messageTypeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    connect(ui.loadProto, &QPushButton::clicked, [this]
        // 弹出文件选择框
        {
            QString path = QFileDialog::getOpenFileName(this, "选择proto文件", "", "proto(*.proto)");
            if (path.isEmpty())
            {
                return;
            }
            loadProto(path);
        });

    // 点击 exportAll，导出全部router的json到文件
    connect(ui.exportAll, &QPushButton::clicked, [this]
        {
            QString path = QFileDialog::getSaveFileName(this, "选择导出文件", "", "json(*.json)");
            if (path.isEmpty())
            {
                return;
            }
            QFile file(path);
            if (!file.open(QIODevice::WriteOnly))
            {
                QMessageBox::warning(this, "错误", "文件打开失败");
                return;
            }

            nlohmann::json j = CommandRouterMap::instance();
            file.write(QByteArray::fromStdString(j.dump(4)));
            file.close();
        });

    // 点击viewAll，预览全体router的json
    connect(ui.viewAll, &QPushButton::clicked, [this]
        {
            nlohmann::json j = CommandRouterMap::instance();
            ui.codeViewer->setText(QString::fromStdString(j.dump(4)));
        });

    // 点击copyAll，复制全体router的json
    connect(ui.copyAll, &QPushButton::clicked, [this]
        {
            QClipboard* clipboard = QApplication::clipboard();
            nlohmann::json j = CommandRouterMap::instance();
            clipboard->setText(QString::fromStdString(j.dump(4)));

            ui.copyAll->setText(tr("Copied"));
            QTimer::singleShot(1000, [this] {
                ui.copyAll->setText(tr("Copy All"));
                });
        });

    connect(ui.messageTypeTable, &QTableWidget::cellChanged, [this](int row, int column)
        // 当表格中的内容发生改变时，更新代码
        {
            if (column != MessageTypeTableColunmIndex::RouterHigh
                && column != MessageTypeTableColunmIndex::RouterLow)
            {
                QMessageBox::warning(this, "警告", "只有RouterHigh和RouterLow可以修改");
                switch (column)
                {
                case MessageTypeTableColunmIndex::Name:
                    ui.messageTypeTable->item(row, column)->setText(messageTypeList_.at(row));
                    break;
                case MessageTypeTableColunmIndex::Type:
                    ui.messageTypeTable->item(row, column)->setText(protoPackage_ + "." + messageTypeList_.at(row));
                    break;
                }
                return;
            }

            auto router_name = ui.messageTypeTable->item(row, MessageTypeTableColunmIndex::Type)->text().toStdString();
            int16_t router_high = ui.messageTypeTable->item(row, MessageTypeTableColunmIndex::RouterHigh)->text().toInt();
            int16_t router_low = ui.messageTypeTable->item(row, MessageTypeTableColunmIndex::RouterLow)->text().toInt();

            CommandRouterMap::instance().append(router_name, router_high, router_low);
            updateCodeViewer(row);
        });

    // 点击单元格，预览生成的json
    connect(ui.messageTypeTable, &QTableWidget::cellClicked, [this](int row, int column)
        {
            if (column != MessageTypeTableColunmIndex::RouterHigh
                && column != MessageTypeTableColunmIndex::RouterLow)
            {
                return;
            }
            updateCodeViewer(row);
        });
}

ProtobufRouterGenerator::~ProtobufRouterGenerator()
{}

void ProtobufRouterGenerator::loadProto(QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    auto content = QString::fromUtf8(file.readAll());
    QRegExp messageRegexp(R"((message\s+)(\w+))");
    QRegExp syntaxRegexp(R"((syntax\W+)(\w+))");
    QRegExp packageRegexp(R"((package\s+)([\w\.]+))");

    protoContent_ = content;
    messageTypeList_.clear();
    protoSyntax_ = "proto2";

    if (syntaxRegexp.indexIn(content) != -1)
    {
        protoSyntax_ = syntaxRegexp.capturedTexts().last();
    }
    if (packageRegexp.indexIn(content) != -1)
    {
        protoPackage_ = packageRegexp.capturedTexts().last();
    }
    messageTypeList_ = regexSearch(messageRegexp, content, {2});

    // 更新RouterMap
    CommandRouterMap::instance().clear();
    for (auto& message : messageTypeList_)
    {
        auto router_name = (protoPackage_ + "." + message).toStdString();
        CommandRouterMap::instance().append(router_name, CommandRouter{});
    }

    // 更新UI

    ui.fileNameLabel->setText(QFileInfo(file).fileName());
    ui.fileNameLabel->setToolTip(QFileInfo(file).fileName());
    ui.fileNameLabel->setStatusTip(QFileInfo(file).canonicalFilePath());
    ui.packageEdit->setText(protoPackage_);
    ui.syntaxEdit->setText(protoSyntax_);

    updateProtoTable();
}

void ProtobufRouterGenerator::updateProtoTable()
{
    ui.messageTypeTable->blockSignals(true);
    ui.messageTypeTable->setRowCount(0);
    for (auto messageType : messageTypeList_)
    {
        auto copyOperateButton = new QPushButton("Copy");
        connect(copyOperateButton, &QPushButton::clicked, [this, messageType, copyOperateButton]
            {
                // 复制此行的router json
                auto router_name = (protoPackage_ + "." + messageType).toStdString();
                auto router = CommandRouterMap::instance().router(router_name, {});
                QClipboard* clipboard = QApplication::clipboard();
                clipboard->setText(QString::fromStdString(nlohmann::json({ {router_name, router} }).dump(4)));

                copyOperateButton->setText(tr("Copied"));
                QTimer::singleShot(1000, [copyOperateButton] {
                    copyOperateButton->setText(tr("Copy"));
                    });
            });

        int row = ui.messageTypeTable->rowCount();
        ui.messageTypeTable->insertRow(row);
        ui.messageTypeTable->setItem(row, MessageTypeTableColunmIndex::Name, new QTableWidgetItem(messageType));
        ui.messageTypeTable->setItem(row, MessageTypeTableColunmIndex::Type, new QTableWidgetItem(protoPackage_ + "." + messageType));
        ui.messageTypeTable->setItem(row, MessageTypeTableColunmIndex::RouterHigh, new QTableWidgetItem(QString::number(0)));
        ui.messageTypeTable->setItem(row, MessageTypeTableColunmIndex::RouterLow, new QTableWidgetItem(QString::number(0)));
        ui.messageTypeTable->setCellWidget(row, MessageTypeTableColunmIndex::Copy, copyOperateButton);
        ui.messageTypeTable->item(row, MessageTypeTableColunmIndex::Name)->setFlags(Qt::ItemIsEnabled);
        ui.messageTypeTable->item(row, MessageTypeTableColunmIndex::Type)->setFlags(Qt::ItemIsEnabled);
    }

    ui.messageTypeTable->blockSignals(false);
}

void ProtobufRouterGenerator::updateCodeViewer(int index)
{
    auto router_name = (protoPackage_ + "." + messageTypeList_.at(index)).toStdString();
    auto router = CommandRouterMap::instance().router(router_name, {});
    ui.codeViewer->setText(QString::fromStdString(nlohmann::json{ {router_name, router} }.dump(4)));
}
