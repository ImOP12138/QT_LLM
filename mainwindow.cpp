#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug> // 用于调试输出

const QString API_KEY = "sk-9QRyHSqEq2fLZfvqwfDQqITPjIXmuxiOtdNgNrUonCwr7loq"; // 替换为你的API密钥
const QString API_URL = "https://chatapi.littlewheat.com/v1/chat/completions";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    networkManager = new QNetworkAccessManager(this);

    // 连接 sendButton 的 clicked() 信号到 on_sendButton_clicked 槽函数
    // 确保信号只连接一次

    //connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::on_sendButton_clicked);
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplyFinished);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendButton_clicked()
{
    qDebug() << "Connecting signals...";
    QString input = ui->inputEdit->toPlainText().trimmed();
    if (input.isEmpty()) {
        qDebug() << "Input is empty. Aborting request.";
        return;
    }

    qDebug() << "User input:" << input;

    // 添加用户消息到历史
    addMessageToHistory("user", input);
    displayMessage("You", input);
    ui->inputEdit->clear();

    // 构造请求JSON
    QJsonObject requestBody;
    requestBody["model"] = "gpt-4"; // 使用 GPT-4 模型
    requestBody["stream"] = false; // 添加 stream 字段

    QJsonArray messagesArray;
    for (const auto& msg : chatHistory) {
        messagesArray.append(msg);
    }
    requestBody["messages"] = messagesArray;

    qDebug() << "Request body:" << QJsonDocument(requestBody).toJson();

    // 设置HTTP请求
    QUrl url(API_URL); // 显式定义 QUrl
    QNetworkRequest request(url); // 使用 QUrl 初始化 QNetworkRequest
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + API_KEY).toUtf8());

    // 将 JSON 数据转换为 QByteArray
    QByteArray postData = QJsonDocument(requestBody).toJson();
    qDebug() << "Sending POST request to URL:" << API_URL;
    qDebug() << "POST data:" << postData;

    // 发送 POST 请求
    QNetworkReply* reply = networkManager->post(request, postData);
    connect(reply, &QNetworkReply::errorOccurred, this, [reply]() {
        qDebug() << "Network error occurred:" << reply->errorString();
    });
}

void MainWindow::onReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network reply error:" << reply->errorString();
        QMessageBox::critical(this, "Error", reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    qDebug() << "Received response from server:" << responseData;

    QJsonDocument response = QJsonDocument::fromJson(responseData);
    if (response.isNull()) {
        qDebug() << "Failed to parse JSON response.";
        QMessageBox::critical(this, "Error", "Failed to parse JSON response.");
        reply->deleteLater();
        return;
    }

    QJsonObject responseObj = response.object();
    if (responseObj.contains("error")) {
        qDebug() << "API returned an error:" << responseObj["error"].toObject()["message"].toString();
        QMessageBox::critical(this, "API Error", responseObj["error"].toObject()["message"].toString());
        reply->deleteLater();
        return;
    }

    QString answer = responseObj["choices"].toArray()[0]
                         .toObject()["message"].toObject()["content"].toString();

    qDebug() << "AI response:" << answer;

    // 添加AI回复到历史
    addMessageToHistory("assistant", answer);
    displayMessage("AI", answer);

    reply->deleteLater();
}

void MainWindow::addMessageToHistory(const QString& role, const QString& content)
{
    QJsonObject message;
    message["role"] = role;
    message["content"] = content;
    chatHistory.append(message);
    qDebug() << "Added message to history:" << message;
}

void MainWindow::displayMessage(const QString& sender, const QString& message)
{
    ui->chatBrowser->append(QString("<b>%1:</b> %2").arg(sender, message));
    qDebug() << "Displayed message:" << sender << ":" << message;
}
