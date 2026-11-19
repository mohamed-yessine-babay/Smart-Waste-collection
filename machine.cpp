#include "machine.h"
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlDatabase>
#include <QPrinter>
#include <QPainter>


Machine::Machine() {
    id_machine = 0;
    capacite = 0;
    type_dechet = "";
    nom_machine = "";
    date_ajout = QDateTime::currentDateTime();
}

Machine::Machine(int id, QString type, QString nom, int capacite, QDateTime date) {
    this->id_machine = id;
    this->type_dechet = type;
    this->nom_machine = nom;
    this->capacite = capacite;
    this->date_ajout = date;
}

bool Machine::ajouter()
{
    qDebug() << "=== DÉBUT AJOUT MACHINE ===";
    qDebug() << "Type déchet :" << type_dechet;
    qDebug() << "Nom machine :" << nom_machine;
    qDebug() << "Capacité    :" << capacite;
    qDebug() << "Date ajout  :" << date_ajout.toString("yyyy-MM-dd HH:mm:ss");

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Connexion non ouverte !" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO MACHINES "
        "(TYPE_DECHET, NOM_MACHINE, CAPACITE, DATE_AJOUT) "
        "VALUES (:type, :nom, :capacite, :date)"
        );

    query.bindValue(":type", type_dechet);
    query.bindValue(":nom", nom_machine);
    query.bindValue(":capacite", capacite);
    query.bindValue(":date", date_ajout);

    if (!query.exec()) {
        qDebug() << "Erreur ajout :" << query.lastError().text();
        qDebug() << "SQL         :" << query.lastQuery();
        return false;
    }

    qDebug() << "Machine ajoutée avec succès";
    return true;
}

QSqlQueryModel* Machine::afficher()
{
    QSqlQueryModel* modelmach = new QSqlQueryModel();

    QSqlQuery query;
    query.prepare(
        "SELECT ID_MACHINE, TYPE_DECHET, NOM_MACHINE, CAPACITE, "
        "TO_CHAR(DATE_AJOUT, 'DD/MM/YYYY HH24:MI:SS') AS DATE_AJOUT "
        "FROM MACHINES "
        "ORDER BY DATE_AJOUT DESC"
        );

    if (!query.exec()) {
        qDebug() << "Erreur affichage :" << query.lastError().text();
        return modelmach;
    }

    modelmach->setQuery(std::move(query));  // ← Utiliser std::move

    modelmach->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    modelmach->setHeaderData(1, Qt::Horizontal, QObject::tr("Type déchet"));
    modelmach->setHeaderData(2, Qt::Horizontal, QObject::tr("Nom machine"));
    modelmach->setHeaderData(3, Qt::Horizontal, QObject::tr("Capacité"));
    modelmach->setHeaderData(4, Qt::Horizontal, QObject::tr("Date ajout"));

    return modelmach;
}

bool Machine::modifier(int id)
{
    qDebug() << "=== MODIFICATION MACHINE ID =" << id << "===";

    QSqlQuery query;
    query.prepare(
        "UPDATE MACHINES SET "
        "TYPE_DECHET = :type, "
        "NOM_MACHINE = :nom, "
        "CAPACITE = :capacite "
        "WHERE ID_MACHINE = :id"
        );

    query.bindValue(":type", type_dechet);
    query.bindValue(":nom", nom_machine);
    query.bindValue(":capacite", capacite);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur modification :" << query.lastError().text();
        return false;
    }

    qDebug() << "Machine modifiée avec succès";
    return true;
}

bool Machine::supprimer(int id)
{
    qDebug() << "=== SUPPRESSION MACHINE ID =" << id << "===";

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Connexion non ouverte !" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM MACHINES WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur suppression :" << query.lastError().text();
        return false;
    }

    qDebug() << "Machine supprimée avec succès";
    return true;
}

QSqlQueryModel* Machine::trierParType(QString type)
{
    QSqlQueryModel* model = new QSqlQueryModel();

    QSqlQuery query;

    if (type.isEmpty() || type == "Tous") {
        query.prepare(
            "SELECT ID_MACHINE, TYPE_DECHET, NOM_MACHINE, CAPACITE, "
            "TO_CHAR(DATE_AJOUT, 'DD/MM/YYYY HH24:MI:SS') AS DATE_AJOUT "
            "FROM MACHINES "
            "ORDER BY TYPE_DECHET ASC"  // ← Tri alphabétique par type
            );
    }

    if (!query.exec()) {
        qDebug() << "Erreur tri :" << query.lastError().text();
        return model;
    }

    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type déchet"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Nom machine"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Capacité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date ajout"));

    return model;
}

QSqlQueryModel* Machine::rechercher(QString nom)
{
    QSqlQueryModel* model = new QSqlQueryModel();

    QSqlQuery query;
    query.prepare(
        "SELECT ID_MACHINE, TYPE_DECHET, NOM_MACHINE, CAPACITE, "
        "TO_CHAR(DATE_AJOUT, 'DD/MM/YYYY HH24:MI:SS') AS DATE_AJOUT "
        "FROM MACHINES "
        "WHERE NOM_MACHINE LIKE :nom "
        "ORDER BY DATE_AJOUT DESC"
        );
    query.bindValue(":nom", "%" + nom + "%");

    if (!query.exec()) {
        qDebug() << "Erreur recherche :" << query.lastError().text();
        return model;
    }

    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type déchet"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Nom machine"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Capacité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date ajout"));

    return model;
}

QMap<QString, int> Machine::getStatsParType() {
    QMap<QString, int> data;
    QSqlQuery query;
    query.exec("SELECT TYPE_DECHET, COUNT(*) FROM MACHINES GROUP BY TYPE_DECHET");
    while (query.next())
        data[query.value(0).toString()] = query.value(1).toInt();
    return data;
}

QMap<QString, double> Machine::getStatsCapacite() {
    QMap<QString, double> stats;
    QSqlQuery query;
    query.exec("SELECT SUM(CAPACITE), AVG(CAPACITE) FROM MACHINES");
    if (query.next()) {
        stats["total"] = query.value(0).toDouble();
        stats["moyenne"] = query.value(1).toDouble();
    }
    return stats;
}

bool Machine::exporterPDF(const QString& cheminFichier)
{
    // 1. Configuration de l'imprimante
    QPrinter printer(QPrinter::ScreenResolution); // Utilisation de la résolution écran pour simplifier le placement
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(cheminFichier);

    QPainter painter;
    if (!painter.begin(&printer)) return false;

    // Récupération de la zone de dessin réelle
    QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();
    int margin = 40;
    int x = margin;
    int y = 60;
    int tableWidth = pageRect.width() - (2 * margin);

    // --- TITRE ---
    painter.setFont(QFont("Arial", 20, QFont::Bold));
    painter.setPen(QColor("#25665A"));
    painter.drawText(pageRect, Qt::AlignTop | Qt::AlignHCenter, "LISTE DES MACHINES");
    y += 80;

    // --- CONFIGURATION COLONNES (en pourcentage de largeur) ---
    // ID (10%), Nom (25%), Type (25%), Capacité (20%), Date (20%)
    QList<int> colWidths = { (int)(tableWidth*0.1), (int)(tableWidth*0.25), (int)(tableWidth*0.25), (int)(tableWidth*0.2), (int)(tableWidth*0.2) };
    QStringList headers = {"ID", "Nom", "Type déchet", "Capacité", "Date ajout"};

    // --- EN-TETE ---
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.setBrush(QColor("#25665A"));
    painter.setPen(Qt::white);

    int rowHeight = 35;
    painter.drawRect(x, y, tableWidth, rowHeight);

    int currentX = x;
    for(int i=0; i<headers.size(); ++i) {
        painter.drawText(currentX + 5, y, colWidths[i], rowHeight, Qt::AlignVCenter, headers[i]);
        currentX += colWidths[i];
    }
    y += rowHeight;

    // --- DONNÉES ---
    QSqlQuery query("SELECT ID_MACHINE, NOM_MACHINE, TYPE_DECHET, CAPACITE, DATE_AJOUT FROM MACHINES");
    painter.setFont(QFont("Arial", 10));
    bool pair = false;

    while (query.next()) {
        // Fond alterné
        painter.setPen(Qt::NoPen);
        painter.setBrush(pair ? QColor("#EAF4F0") : Qt::white);
        painter.drawRect(x, y, tableWidth, rowHeight);
        pair = !pair;

        painter.setPen(Qt::black);
        currentX = x;

        // Dessin de chaque cellule
        for(int i=0; i<5; ++i) {
            QString val = (i == 4) ? query.value(i).toDateTime().toString("dd/MM/yyyy") : query.value(i).toString();
            painter.drawText(currentX + 5, y, colWidths[i], rowHeight, Qt::AlignVCenter, val);
            currentX += colWidths[i];
        }

        y += rowHeight;

        // Gestion fin de page (simple)
        if (y > pageRect.height() - 50) {
            printer.newPage();
            y = margin;
        }
    }

    painter.end();
    return true;
}
// ********************************* Recyclage *******************************************

