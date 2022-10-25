#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ProtobufRouterGenerator.h"

class ProtobufRouterGenerator : public QMainWindow
{
    Q_OBJECT

public:
    ProtobufRouterGenerator(QWidget *parent = nullptr);
    ~ProtobufRouterGenerator();

	void loadProto(QString path);

	static inline decltype(auto) regexSearch(QRegExp& regex, const QString& content, const QList<int>& indexs = {}) {
		QStringList machList{};
		int pos = 0;
		while ((pos = regex.indexIn(content, pos)) != -1) {
			pos += regex.matchedLength();
			if (indexs.IsEmpty())
			{
				machList += regex.capturedTexts();
			}
			else
			{
				for(auto index : indexs)
				{
					machList += regex.capturedTexts().at(index);
				}
			}
		}
		return machList;
	};

private:
    Ui::ProtobufRouterGeneratorClass ui;

	QString protoContent_{};
	QString protoSyntax_{ "proto2" };
	QString protoPackage_{"pb"};
	QStringList messageTypeList_{};
};
