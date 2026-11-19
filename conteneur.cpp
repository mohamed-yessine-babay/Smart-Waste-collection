#include "conteneur.h"
#include "smtpclient.h"
#include <QMessageBox>
#include <cmath>

// Constructors
Conteneur::Conteneur()
{
    id_conteneur = 0;
    capacite_max = 0;
    quantite_actuelle = 0;
    seuil_alerte = 0;
    date_maj = QDate::currentDate();
}

Conteneur::Conteneur(int id, QString t, int cap, int qte, int seuil, QDate date)
{
    this->id_conteneur = id;
    this->type = t;
    this->capacite_max = cap;
    this->quantite_actuelle = qte;
    this->seuil_alerte = seuil;
    this->date_maj = date;
}

// Calculated fields
int Conteneur::getPourcentageRemplissage() const
{
    if (capacite_max <= 0) return 0;
    return (quantite_actuelle * 100) / capacite_max;
}

QString Conteneur::getNiveauAlerte() const
{
    int pourcentage = getPourcentageRemplissage();
    if (pourcentage >= 90) {
        return "Critique";
    } else if (pourcentage >= seuil_alerte) {
        return "Avertissement";
    } else {
        return "Normal";
    }
}

// Calculate fill rate per day based on current fill level and days since last update
double Conteneur::getFillRatePerDay() const
{
    // If container is empty or at 100%, return 0
    if (quantite_actuelle <= 0 || quantite_actuelle >= capacite_max) {
        return 0;
    }

    // Calculate days since last update
    QDate currentDate = QDate::currentDate();
    int daysSinceUpdate = date_maj.daysTo(currentDate);

    // If updated today or no time passed, use a default rate based on type
    if (daysSinceUpdate <= 0) {
        // Default daily fill rates based on container type (in liters per day)
        if (type == "Plastique") return 50;
        else if (type == "Verre") return 30;
        else if (type == "Papier") return 70;
        else if (type == "Métal") return 40;
        else if (type == "Organique") return 60;
        else return 45; // Default rate
    }

    // Calculate fill rate (liters per day)
    double fillRate = (double)quantite_actuelle / daysSinceUpdate;

    return fillRate;
}

// Calculate days until container reaches full capacity
int Conteneur::getDaysUntilFull() const
{
    if (quantite_actuelle >= capacite_max) {
        return 0; // Already full
    }

    double fillRate = getFillRatePerDay();

    if (fillRate <= 0) {
        return -1; // No activity (can't predict)
    }

    int remainingCapacity = capacite_max - quantite_actuelle;
    int daysUntilFull = (int)ceil(remainingCapacity / fillRate);

    return daysUntilFull;
}

// Get time until full
QString Conteneur::getTimeUntilFull() const
{
    if (quantite_actuelle >= capacite_max) {
        return "Deja plein";
    }

    int days = getDaysUntilFull();

    if (days < 0) {
        return "Non disponible";
    }

    if (days == 0) {
        return "Aujourd'hui";
    }

    if (days == 1) {
        return "Demain";
    }

    if (days < 7) {
        return QString("Dans %1 jours").arg(days);
    }

    int weeks = days / 7;
    int remainingDays = days % 7;

    if (remainingDays == 0) {
        return QString("Dans %1 semaine(s)").arg(weeks);
    } else {
        return QString("Dans %1 semaine(s) et %2 jours").arg(weeks).arg(remainingDays);
    }
}

// Vérifier si un ID existe déjà
bool Conteneur::idExiste(int id)
{
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM CONTENEURS WHERE ID_CONTENEUR = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt() > 0;
        }
    } else {
        qDebug() << "Error in idExiste:" << query.lastError().text();
    }

    return false;
}

// NEW: Get all containers
QVector<Conteneur> Conteneur::getAllConteneurs()
{
    QVector<Conteneur> conteneurs;
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return conteneurs;
    }

    QSqlQuery query(db);
    query.prepare("SELECT * FROM CONTENEURS ORDER BY ID_CONTENEUR");

    if (!query.exec()) {
        qDebug() << "Error fetching containers:" << query.lastError().text();
        return conteneurs;
    }

    while (query.next()) {
        Conteneur c(
            query.value("ID_CONTENEUR").toInt(),
            query.value("TYPE").toString(),
            query.value("CAPACITE_MAX").toInt(),
            query.value("QUANTITE_ACTUELLE").toInt(),
            query.value("SEUIL_ALERTE").toInt(),
            query.value("DATE_MAJ").toDate()
            );
        conteneurs.append(c);
    }

    return conteneurs;
}
// Add this function to conteneur.cpp
QString Conteneur::getFormattedDataForArduino(int id)
{
    // Query the database for the container
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return "";
    }

    QSqlQuery query(db);
    query.prepare("SELECT ID_CONTENEUR, TYPE, CAPACITE_MAX, QUANTITE_ACTUELLE FROM CONTENEURS WHERE ID_CONTENEUR = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Query error:" << query.lastError().text();
        return "";
    }

    if (query.next()) {
        // Get data from query result
        int containerId = query.value("ID_CONTENEUR").toInt();
        QString type = query.value("TYPE").toString();
        int capaciteMax = query.value("CAPACITE_MAX").toInt();
        int quantiteActuelle = query.value("QUANTITE_ACTUELLE").toInt();

        // Calculate fill percentage
        int fillPercentage = 0;
        if (capaciteMax > 0) {
            fillPercentage = (quantiteActuelle * 100) / capaciteMax;
        }

        // Remove special characters from type
        type.replace("é", "e");
        type.replace("è", "e");
        type.replace("ê", "e");
        type.replace("ë", "e");
        type.replace("à", "a");
        type.replace("â", "a");
        type.replace("ä", "a");
        type.replace("ô", "o");
        type.replace("ö", "o");
        type.replace("ù", "u");
        type.replace("û", "u");
        type.replace("ü", "u");
        type.replace("ç", "c");
        type.replace("ï", "i");
        type.replace("î", "i");

        // Format for Arduino: "ID:123|TYPE:Plastique|FILL:75%\n"
        QString formattedData = QString("ID:%1|TYPE:%2|FILL:%3%\n")
                                    .arg(containerId)
                                    .arg(type)
                                    .arg(fillPercentage);

        qDebug() << "Formatted data for Arduino:" << formattedData;
        return formattedData;
    }

    qDebug() << "Container not found with ID:" << id;
    return "";
}
// NEW: Get container by ID
Conteneur Conteneur::getConteneurById(int id)
{
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return Conteneur();
    }

    QSqlQuery query(db);
    query.prepare("SELECT * FROM CONTENEURS WHERE ID_CONTENEUR = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return Conteneur(
            query.value("ID_CONTENEUR").toInt(),
            query.value("TYPE").toString(),
            query.value("CAPACITE_MAX").toInt(),
            query.value("QUANTITE_ACTUELLE").toInt(),
            query.value("SEUIL_ALERTE").toInt(),
            query.value("DATE_MAJ").toDate()
            );
    }

    return Conteneur();
}

// NEW: Search containers by text
QVector<Conteneur> Conteneur::rechercherConteneurs(const QString& searchText)
{
    QVector<Conteneur> conteneurs;
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return conteneurs;
    }

    QSqlQuery query(db);
    QString queryStr = "SELECT * FROM CONTENEURS WHERE "
                       "LOWER(TYPE) LIKE LOWER('%" + searchText + "%') OR "
                                      "TO_CHAR(ID_CONTENEUR) LIKE '%" + searchText + "%' "
                                      "ORDER BY ID_CONTENEUR";
    query.prepare(queryStr);

    if (query.exec()) {
        while (query.next()) {
            Conteneur c(
                query.value("ID_CONTENEUR").toInt(),
                query.value("TYPE").toString(),
                query.value("CAPACITE_MAX").toInt(),
                query.value("QUANTITE_ACTUELLE").toInt(),
                query.value("SEUIL_ALERTE").toInt(),
                query.value("DATE_MAJ").toDate()
                );
            conteneurs.append(c);
        }
    }

    return conteneurs;
}

// NEW: Filter containers by type
QVector<Conteneur> Conteneur::filtrerParType(const QString& type)
{
    QVector<Conteneur> conteneurs;
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return conteneurs;
    }

    QSqlQuery query(db);
    QString queryStr = "SELECT * FROM CONTENEURS WHERE TYPE = '" + type + "' ORDER BY ID_CONTENEUR";
    query.prepare(queryStr);

    if (query.exec()) {
        while (query.next()) {
            Conteneur c(
                query.value("ID_CONTENEUR").toInt(),
                query.value("TYPE").toString(),
                query.value("CAPACITE_MAX").toInt(),
                query.value("QUANTITE_ACTUELLE").toInt(),
                query.value("SEUIL_ALERTE").toInt(),
                query.value("DATE_MAJ").toDate()
                );
            conteneurs.append(c);
        }
    }

    return conteneurs;
}

bool Conteneur::ajouter()
{
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO CONTENEURS (ID_CONTENEUR, TYPE, CAPACITE_MAX, QUANTITE_ACTUELLE, SEUIL_ALERTE, DATE_MAJ) "
                  "VALUES (:id, :type, :capacite, :quantite, :seuil, :date)");

    query.bindValue(":id", id_conteneur);
    query.bindValue(":type", type);
    query.bindValue(":capacite", capacite_max);
    query.bindValue(":quantite", quantite_actuelle);
    query.bindValue(":seuil", seuil_alerte);
    query.bindValue(":date", date_maj);

    bool result = query.exec();

    if (!result) {
        qDebug() << "Error in ajouter:" << query.lastError().text();
        return false;
    }

    // ✅ CALL CENTRALIZED EMAIL FUNCTION
    handleEmail(db);

    return true;
}

// READ - AFFICHER (PDF page 40)
QSqlQueryModel* Conteneur::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM CONTENEURS ORDER BY ID_CONTENEUR");

    if (model->lastError().isValid()) {
        qDebug() << "Error in afficher:" << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Capacité Max"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Quantité Actuelle"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Seuil d'Alerte"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Date Mise à Jour"));

    return model;
}

bool Conteneur::modifier(int id)
{
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE CONTENEURS SET TYPE = :type, CAPACITE_MAX = :capacite, "
                  "QUANTITE_ACTUELLE = :quantite, SEUIL_ALERTE = :seuil, DATE_MAJ = :date "
                  "WHERE ID_CONTENEUR = :id");

    query.bindValue(":id", id);
    query.bindValue(":type", type);
    query.bindValue(":capacite", capacite_max);
    query.bindValue(":quantite", quantite_actuelle);
    query.bindValue(":seuil", seuil_alerte);
    query.bindValue(":date", date_maj);

    bool result = query.exec();

    if (!result) {
        qDebug() << "Error in modifier:" << query.lastError().text();
        return false;
    }

    // ✅ ALSO SEND EMAIL HERE
    handleEmail(db);

    return true;
}

void Conteneur::handleEmail(QSqlDatabase db)
{
    QSqlQuery q(db);

    q.prepare("SELECT email FROM employee");

    if (!q.exec()) {
        qDebug() << "Email query error:" << q.lastError().text();
        return;
    }

    if (!q.next()) {
        qDebug() << "No chef found!";
        QMessageBox::warning(nullptr, "Email", "Chef email not found!");
        return;
    }

    QString chefEmail = q.value(0).toString();
    qDebug() << "Chef email:" << chefEmail;

    QString status;
    if (quantite_actuelle >= capacite_max)
        status = "FULL";
    else if (quantite_actuelle >= capacite_max / 2)
        status = "MEDIUM";
    else
        status = "SAFE";

    QString subject = "Alerte Conteneur";

    QString body =
        "Conteneur ID: " + QString::number(id_conteneur) + "\n"
                                                           "Type: " + type + "\n"
                 "Remplissage: " + QString::number(getPourcentageRemplissage()) + "%\n\n";

    if (status == "FULL")
        body += "⚠️ Le conteneur est plein (100%)";
    else if (status == "MEDIUM")
        body += "⚠️ Le conteneur est à moitié plein";
    else
        body += "✅ Le conteneur est dans un état sûr";

    bool emailSent = SmtpClient::sendMail(chefEmail, subject, body);

    if (emailSent)
        QMessageBox::information(nullptr, "Succès", "Email envoyé !");
    else
        QMessageBox::critical(nullptr, "Erreur", "Échec d'envoi !");
}

// DELETE - SUPPRIMER (PDF page 36)
bool Conteneur::supprimer(int id)
{
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM CONTENEURS WHERE ID_CONTENEUR = :id");
    query.bindValue(":id", id);

    return query.exec();
}

void Conteneur::clear()
{
    id_conteneur = 0;
    type = "";
    capacite_max = 0;
    quantite_actuelle = 0;
    seuil_alerte = 0;
    date_maj = QDate::currentDate();
}
// NEW: Get container statistics
Conteneur::ContainerStats Conteneur::getStatistics()
{
    ContainerStats stats;

    // Initialize counts
    stats.total = 0;
    stats.empty = 0;
    stats.quarterFull = 0;
    stats.halfFull = 0;
    stats.threeQuarterFull = 0;
    stats.almostFull = 0;
    stats.full = 0;

    // Get all containers
    QVector<Conteneur> conteneurs = getAllConteneurs();
    stats.total = conteneurs.size();

    if (stats.total == 0) {
        // Set all percentages to 0 if no containers
        stats.emptyPercent = 0;
        stats.quarterFullPercent = 0;
        stats.halfFullPercent = 0;
        stats.threeQuarterFullPercent = 0;
        stats.almostFullPercent = 0;
        stats.fullPercent = 0;
        return stats;
    }

    // Categorize each container
    for (const Conteneur& c : conteneurs) {
        int percent = c.getPourcentageRemplissage();

        if (percent == 0) {
            stats.empty++;
        } else if (percent <= 25) {
            stats.quarterFull++;
        } else if (percent <= 50) {
            stats.halfFull++;
        } else if (percent <= 75) {
            stats.threeQuarterFull++;
        } else if (percent < 100) {
            stats.almostFull++;
        } else {
            stats.full++;
        }
    }

    // Calculate percentages
    stats.emptyPercent = (double)stats.empty / stats.total * 100;
    stats.quarterFullPercent = (double)stats.quarterFull / stats.total * 100;
    stats.halfFullPercent = (double)stats.halfFull / stats.total * 100;
    stats.threeQuarterFullPercent = (double)stats.threeQuarterFull / stats.total * 100;
    stats.almostFullPercent = (double)stats.almostFull / stats.total * 100;
    stats.fullPercent = (double)stats.full / stats.total * 100;

    return stats;
}
