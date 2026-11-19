#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QString>

class SmtpClient {
public:
    static bool sendMail(const QString &to,
                         const QString &subject,
                         const QString &body);
};

#endif
