#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* networkManager;
    QVector<QJsonObject> chatHistory; // 维护对话历史

    void addMessageToHistory(const QString& role, const QString& content);
    void displayMessage(const QString& sender, const QString& message);

private slots:
    void on_sendButton_clicked();
    void onReplyFinished(QNetworkReply* reply);


};
#endif // MAINWINDOW_H
