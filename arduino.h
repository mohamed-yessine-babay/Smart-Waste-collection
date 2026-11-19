    #ifndef ARDUINO_H
#define ARDUINO_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QString>
#include <QObject>
#include <QDebug>

class Arduino : public QObject
{
    Q_OBJECT

private:
    QByteArray m_buffer;
    QSerialPort *serial;
    QString arduino_port_name;
    bool arduino_is_available;
    static const quint16 arduino_uno_vendor_id = 9025;
    static const quint16 arduino_uno_product_id = 67;
    QByteArray data; // contenant les données lues à partir d'Arduino
    // organistation des signaux
    enum idxGestion{
        GEMPLOYE = 0,
        GCONTAINER = 1,
        GMACHINE = 2,
        GCAMION = 3,
        GPOINT_COLLECTE = 4,
        GCLIENT = 5
    };
public:
    explicit Arduino(QObject *parent = nullptr);
    int connect_arduino();
    int close_arduino();
    QSerialPort* getserial();
    QString getarduino_port_name();
    void demanderMesure();
    QByteArray read_from_arduino();
    void write_to_arduino(QByteArray);
    void send_remplissage(float);
    void envoyerSignal(const QString &commande);
    // Separateur arduino
    void onDonneesRecues();
    void sendToArduino(int id, const QString& type, int fillPercentage);

    void identifyArduino();

signals:
    void arduinoPret();
    void enregistrementDebute();
    void enregistrementTermine(int duree);
    void erreurConnexion(const QString &msg);

    // signal pour la coordination arduino
    void gEmpArd(QByteArray b);
    void gMachArd(QByteArray b);
    void gCliArd(QByteArray b);
    void gPcArd(QByteArray b);
    void gConArd(QByteArray b);
    void gCamArd(QByteArray b);

private slots:

private:
    void traiterMessage(const QString &message);
};

#endif // ARDUINO_H
