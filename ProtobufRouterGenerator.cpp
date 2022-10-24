#include "ProtobufRouterGenerator.h"
#include <QFile>
#include <QFileDialog>
#include <QRegExp>
#include <QPushButton>

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

	protoSyntax_ = regexSearch(syntaxRegexp, content, 2).first();
	protoPackage_ = regexSearch(packageRegexp, content, 2).first();
	messageTypeList_ = regexSearch(messageRegexp, content, 2);

	ui.fileNameLabel->setText(QFileInfo(file).fileName());
	ui.fileNameLabel->setToolTip(QFileInfo(file).fileName());
	ui.fileNameLabel->setStatusTip(QFileInfo(file).canonicalFilePath());
	ui.packageEdit->setText(protoPackage_);
	ui.syntaxEdit->setText(protoSyntax_);

	ui.messageTypeTable->setRowCount(0);
	for (auto messageType : messageTypeList_)
	{
		int row = ui.messageTypeTable->rowCount();
		ui.messageTypeTable->insertRow(row);
		ui.messageTypeTable->setItem(row, 0, new QTableWidgetItem(messageType));
		ui.messageTypeTable->setItem(row, 1, new QTableWidgetItem(protoPackage_ + "." + messageType));
		ui.messageTypeTable->setItem(row, 2, new QTableWidgetItem(QString::number(0)));
		ui.messageTypeTable->setItem(row, 3, new QTableWidgetItem(QString::number(0)));
	}
}
