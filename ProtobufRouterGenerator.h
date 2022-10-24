#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ProtobufRouterGenerator.h"

class ProtobufRouterGenerator : public QMainWindow
{
    Q_OBJECT

public:
    ProtobufRouterGenerator(QWidget *parent = nullptr);
    ~ProtobufRouterGenerator();

private:
    Ui::ProtobufRouterGeneratorClass ui;
};
