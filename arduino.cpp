#include "arduino.h"

Arduino::Arduino(QObject *parent): QObject(parent) {

    serial= new QSerialPort(this);
    arduino_is_available = false;
    arduino_port_name="";

}

int Arduino::connect_arduino() {
    // Debug
    qDebug() << "=== Ports disponibles ===";
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        qDebug() << "Port:" << info.portName()
        << "| VID:" << Qt::hex << info.vendorIdentifier()
        << "| PID:" << Qt::hex << info.productIdentifier()
        << "| Desc:" << info.description();
    }
    qDebug() << "=========================";

    // ← Boucle de détection — c'était ça qui manquait
    foreach(const QSerialPortInfo &serial_port_info, QSerialPortInfo::availablePorts()) {
        if(serial_port_info.hasVendorIdentifier() && serial_port_info.hasProductIdentifier()) {
            if(serial_port_info.vendorIdentifier() == arduino_uno_vendor_id &&
                serial_port_info.productIdentifier() == arduino_uno_product_id) {
                arduino_is_available = true;
                arduino_port_name = serial_port_info.portName();
            }
        }
    }

    qDebug() << "arduino_port_name is :" << arduino_port_name;

    if(arduino_is_available) {
        serial->setPortName(arduino_port_name);
        if(serial->open(QSerialPort::ReadWrite)) {
            serial->setBaudRate(QSerialPort::Baud9600);
            serial->setDataBits(QSerialPort::Data8);
            serial->setParity(QSerialPort::NoParity);
            serial->setStopBits(QSerialPort::OneStop);
            serial->setFlowControl(QSerialPort::NoFlowControl);
            return 0;
        }
        return 1;
    }
    return -1;
}


int Arduino::close_arduino(){
    if (serial->isOpen()){
        serial->close();
        return 0;
    }
    return 1;
}

QString Arduino::getarduino_port_name(){
    return arduino_port_name;
}

QSerialPort* Arduino::getserial(){
    return serial ;
}

void Arduino::demanderMesure()
{
    if(serial->isWritable()){
        serial->write("M"); //envoyer des donnes vers arduino
    }else{
        qDebug() << "Couldn't write to serial !";
    }

}
QByteArray Arduino::read_from_arduino()
{
    if (serial->isOpen())
    {
        return serial->readAll();
    }

    return QByteArray();
}

void Arduino::write_to_arduino( QByteArray d)
{

    if(serial->isWritable()){
        serial->write(d);  // envoyer des donnés vers Arduino
    }else{
        qDebug() << "Couldn't write to serial!";
    }
}

void Arduino::send_remplissage(float remplissage)
{
    float remp_form = remplissage*10;
    int remp_format  = (int)remp_form;
    QString formatted;

    if (remp_format >= 1000)
        formatted = "000";
    else
        formatted = QString("%1\n").arg(remp_format, 3, 10, QChar('0'));
    qDebug() << formatted;
    Arduino::write_to_arduino(formatted.toUtf8());
}
void Arduino::envoyerSignal(const QString &commande)
{
    if (!serial->isOpen()) {
        qDebug() << "[Arduino] Port fermé — signal ignoré :" << commande;
        return;
    }
    QByteArray data = (commande + "\n").toUtf8();
    Arduino::write_to_arduino(data);
    qDebug() << "[Arduino] Signal envoyé :" << commande;
}
void Arduino::onDonneesRecues()
{
    m_buffer += serial->readAll();
    while (m_buffer.contains('\n')) {
        int idx       = m_buffer.indexOf('\n');
        QString ligne = m_buffer.left(idx).trimmed();
        m_buffer      = m_buffer.mid(idx + 1);
        if (!ligne.isEmpty()) {
            qDebug() << "[Arduino] Reçu :" << ligne;
            traiterMessage(ligne);
        }
    }
}

void Arduino::traiterMessage(const QString &message)
{
    if (message == "STATUS:READY") {
        emit arduinoPret();
    } else if (message == "STATUS:REC_START") {
        emit enregistrementDebute();
    } else if (message.startsWith("REC:DONE:")) {
        QStringList parts = message.split(':');
        bool ok;
        int duree = parts.size() >= 3 ? parts[2].toInt(&ok) : 0;
        if (ok) emit enregistrementTermine(duree);
        else qWarning() << "[Arduino] Format invalide :" << message;
    }
}


void Arduino::identifyArduino(){
    m_buffer += Arduino::read_from_arduino();
    if(m_buffer.contains('\n')){
        int idx = m_buffer.indexOf('\n');
        QByteArray ligne = m_buffer.left(idx).trimmed();
        m_buffer = m_buffer.mid(idx+1);

        int separateur = ligne.indexOf(':');
        if(separateur != -1){
            bool okId;
            int id = ligne.left(separateur).toInt(&okId);
            QByteArray valeur = ligne.mid(separateur + 1);  // tout après le ':'
            if(okId){
                switch(id){
                case GEMPLOYE:
                    emit gEmpArd(valeur);
                    break;
                case GMACHINE:
                    emit gMachArd(valeur);
                    break;
                case GCLIENT:
                    emit gCliArd(valeur);
                    break;
                case GPOINT_COLLECTE:
                    emit gPcArd(valeur);
                    break;
                case GCONTAINER:
                    emit gConArd(valeur);
                    break;
                case GCAMION:
                    emit gCamArd(valeur);
                    break;
                }
            }
        }
    }
}

void Arduino::sendToArduino(int id, const QString& type, int fillPercentage)
{
    if (!serial->isOpen()) {
        qDebug() << "Cannot send - Arduino not connected";
        return;
    }
    // Clean the type (remove special characters)
    QString cleanType = type;
    cleanType.replace("é", "e");
    cleanType.replace("è", "e");
    cleanType.replace("ê", "e");
    cleanType.replace("ë", "e");
    cleanType.replace("à", "a");
    cleanType.replace("â", "a");
    cleanType.replace("ä", "a");
    cleanType.replace("ô", "o");
    cleanType.replace("ö", "o");
    cleanType.replace("ù", "u");
    cleanType.replace("û", "u");
    cleanType.replace("ü", "u");
    cleanType.replace("ç", "c");
    cleanType.replace("ï", "i");
    cleanType.replace("î", "i");

    // Format: "ID:123|TYPE:Plastique|FILL:75%\n"
    QString data = QString("ID:%1|TYPE:%2|FILL:%3%\n")
                       .arg(id)
                       .arg(cleanType)
                       .arg(fillPercentage);

    QByteArray bytes = data.toUtf8();
    //qint64 bytesWritten = arduinoSerial->write(bytes);
    qDebug() << "Sent to Arduino:" << data.trimmed();
    Arduino::write_to_arduino(bytes);
    /*
    if (bytesWritten == -1) {
        qDebug() << "Failed to write to serial port";

    } else {
        arduinoSerial->flush();

    }*/
}
