#include "smtpclient.h"
#include <QTcpSocket>
#include <QSslSocket>

bool SmtpClient::sendMail(const QString &to,
                          const QString &subject,
                          const QString &body)
{
    QSslSocket socket;
    socket.connectToHostEncrypted("smtp.gmail.com", 465);

    if (!socket.waitForConnected(5000))
        return false;

    if (!socket.waitForReadyRead(5000))
        return false;

    auto send = [&](QString cmd) {
        socket.write(cmd.toUtf8() + "\r\n");
        socket.flush();
        socket.waitForReadyRead(3000);
    };

    QString username = "simawiyass124@gmail.com";
    QString password = "ptqw ddbt cuor zjha";

    send("EHLO localhost");
    send("AUTH LOGIN");

    send(QByteArray(username.toUtf8()).toBase64());
    send(QByteArray(password.toUtf8()).toBase64());

    send("MAIL FROM:<" + username + ">");
    send("RCPT TO:<" + to + ">");
    send("DATA");

    QString msg =
        "Subject: " + subject + "\r\n"
                                "\r\n"
        + body + "\r\n.";

    send(msg);
    send("QUIT");

    socket.disconnectFromHost();
    return true;
}
