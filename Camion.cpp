#include "Camion.h"


Camion::Camion() {
    this->matricule = "";
    this->capacite_max = 0;
    this->remplissage = 0;
    this->consomation = 0;
    this->status = "";
    this->camEditMode=false;
    this->trieAsc = true;
}

Camion::Camion(QString matricule, float capacite_max, float remplissage, float consomation, QString status)
{
    this->matricule = matricule;
    this->capacite_max = capacite_max;
    this->remplissage = remplissage;
    this->consomation = consomation;
    this->status = status;
    this->camEditMode=false;
    this->trieAsc = true;
}

bool Camion::addCamion()const
{
    QSqlQuery query;
    query.prepare("insert into CAMIONS (matricule, capacite_max, remplissage, consomation, status) "
                  "values (:matricule, :capacite_max, :remplissage, :consomation, :status)");

    query.bindValue(":matricule",matricule);
    query.bindValue(":capacite_max",capacite_max);
    query.bindValue(":remplissage",remplissage);
    query.bindValue(":consomation",consomation);
    query.bindValue(":status",status);
    return query.exec();
}
QSqlQueryModel * Camion::showCamion()const
{
    QSqlQueryModel * model= new QSqlQueryModel();
    model->setQuery("select matricule, capacite_max, remplissage, consomation, status from CAMIONS");
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("Matricule"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Capacité max"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Remplissage"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Consomation"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Status"));

    return model;
}
bool Camion::editCamion()const
{
    QSqlQuery query;
    query.prepare(
        "UPDATE CAMIONS SET "
        "matricule = :matricule, "
        "capacite_max = :capacite_max, "
        "remplissage = :remplissage, "
        "consomation = :consomation, "
        "status = :status  "
        "where id_camion = :id_camion"
        );
    query.bindValue(":id_camion",id_camion);
    query.bindValue(":matricule",matricule);
    query.bindValue(":capacite_max",capacite_max);
    query.bindValue(":remplissage",remplissage);
    query.bindValue(":consomation",consomation);
    query.bindValue(":status",status);
    return query.exec();
}
bool Camion::deleteCamion(QString matricule)const
{
    QSqlQuery query;
    query.prepare("Delete from CAMIONS where MATRICULE= :matricule");
    query.bindValue(":matricule", matricule);
    return query.exec();
}

bool Camion::fetchCamionByMatricule(QString matricule)
{
    QSqlQuery query;
    query.prepare("Select * from CAMIONS where matricule = :matricule");
    query.bindValue(":matricule",matricule);
    query.exec();

    if(query.isActive() && !query.next())
    {
        return false;
    }
    this->id_camion = query.value("id_camion").toInt();
    this->matricule = query.value("matricule").toString();
    this->capacite_max = query.value("capacite_max").toFloat();
    this->remplissage = query.value("remplissage").toFloat();
    this->consomation = query.value("consomation").toFloat();
    this->status = query.value("status").toString();
    return true;
}

bool Camion::fetchCamionById(int id)
{
    QSqlQuery query;
    query.prepare("Select * from CAMIONS where id_camion = :id");
    query.bindValue(":id",id);
    query.exec();

    if(query.isActive() && !query.next())
    {
        return false;
    }
    this->id_camion = query.value("id_camion").toInt();
    this->matricule = query.value("matricule").toString();
    this->capacite_max = query.value("capacite_max").toFloat();
    this->remplissage = query.value("remplissage").toFloat();
    this->consomation = query.value("consomation").toFloat();
    this->status = query.value("status").toString();
    return true;
}

bool Camion::camControleSaisie()const
{
    if(capacite_max <= 0 || capacite_max > 70000)
    {
        QMessageBox::critical(
            nullptr,
            QMessageBox::tr("Erreur Saisie"),
            QMessageBox::tr("La capacite maximale saisie est invalide")
            );
        return false;
    }else if(consomation <=0 || consomation >100)
    {
        QMessageBox::critical(
            nullptr,
            QMessageBox::tr("Erreur Saisie"),
            QMessageBox::tr("La Consomation saisie est invalide")
            );
        return false;
    }else if(remplissage > capacite_max)
    {
        QMessageBox::critical(
            nullptr,
            QMessageBox::tr("Erreur Saisie"),
            QMessageBox::tr("Le Remplissage saisie est invalide")
            );
        return false;
    }
    return true;
}

QSqlQueryModel * Camion::camRechMat(QString Matricule)const
{
    QSqlQueryModel * model= new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT matricule, capacite_max, remplissage, consomation, status FROM CAMIONS "
                  "WHERE UPPER(matricule) LIKE UPPER(:matricule)");
    query.bindValue(":matricule", "%"+Matricule+"%");
    if(!query.exec())
    {
        qDebug() << "Erreur SQL:" << query.lastError().text();
    }
    model->setQuery(std::move(query));
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("Matricule"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Capacité max"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Remplissage"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Consomation"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Status"));

    return model;
}
QSqlQueryModel * Camion::camTrierStatus(QString Matricule)const
{
    QString ordre = trieAsc ? "ASC" : "DESC";
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery query;
    if(Matricule.trimmed().isEmpty())
    {
        query.prepare(QString("SELECT matricule, capacite_max, remplissage, consomation, status "
                              "FROM CAMIONS "
                              "ORDER BY status %1").arg(ordre));
    }else
    {
        query.prepare(QString("SELECT matricule, capacite_max, remplissage, consomation, status "
                              "FROM CAMIONS "
                              "WHERE UPPER(matricule) LIKE UPPER(:matricule) "
                              "ORDER BY status %1").arg(ordre));
        query.bindValue(":matricule", "%"+Matricule+"%");
    }
    if(!query.exec())
    {
        qDebug() << "Erreur SQL:"<< query.lastError().text();
    }
    model->setQuery(std::move(query));
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("Matricule"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Capacité max"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Remplissage"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Consomation"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Status"));

    return model;
}


void Camion::camExportPDF(QSqlQueryModel *model)const
{
    QString chemFichCamExp = QFileDialog::getSaveFileName(
        nullptr,
        "Exporter en PDF",
        "Camions.pdf",
        "PDF files (*.pdf)"
        );
    if(chemFichCamExp.isEmpty()) return;
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(chemFichCamExp);
    printer.setPageOrientation(QPageLayout::Portrait);


    // Mise en page du PDF
    QPainter painter;
    if(!painter.begin(&printer))
    {
        qDebug() << "Erreur: Impossible d'ouvrir le fichier PDF";
        return;
    }

    int pageWidth = printer.pageRect(QPrinter::DevicePixel).width();
    int pageHeight = printer.pageRect(QPrinter::DevicePixel).height();
    int margin = 400;
    int colCount = model->columnCount();
    int rowHeight = 300;
    int colWidth = (pageWidth - 2 * margin)/colCount;
    int y = margin;

    QFont policeTitre("Arial", 14, QFont::Bold);
    painter.setFont(policeTitre);
    painter.drawText(margin, y+100, "Liste des Camions");
    y += 200;

    QFont policeEntete("Arial", 9, QFont::Bold);
    painter.setFont(policeEntete);
    painter.setBrush(QColor(41,128,185));
    painter.setPen(Qt::black);

    for(int col = 0; col < colCount; col++)
    {
        QRect rect(margin + col * colWidth, y , colWidth, rowHeight);
        painter.fillRect(rect, QColor(41,128,185));
        painter.drawRect(rect);
        painter.drawText(rect,Qt::AlignCenter,
                         model->headerData(col, Qt::Horizontal).toString());
    }
    y += rowHeight;

    QFont policeIndex("Arial", 8);
    painter.setFont(policeIndex);
    painter.setPen(Qt::black);

    for(int row = 0;row < model->rowCount(); row++)
    {
        if(y + rowHeight > pageHeight - margin)
        {
            printer.newPage();
            y=margin;
        }

        QColor bgColor = (row % 2 == 0)? Qt::white : QColor(236, 240, 241);
        painter.setBrush(bgColor);
        for( int col = 0; col < colCount; col++)
        {
            QRect rect1(margin + col *colWidth, y, colWidth, rowHeight);
            painter.fillRect(rect1, Qt::white);
            painter.drawRect(rect1);
            QString valeur = model->data(model->index(row, col)).toString();

            // Si c'est un nombre, limiter à 2 décimales
            bool isNumber;
            float nombre = valeur.toFloat(&isNumber);
            if (isNumber) {
                valeur = QString::number(nombre, 'f', 2); // ← 2 décimales
            }
            painter.drawText(rect1, Qt::AlignCenter,valeur);
        }
        y += rowHeight;
    }
    painter.end();
    QMessageBox::information(
        nullptr,
        QMessageBox::tr("Eportation PDF"),
        "Exportation pdf réussie à l'emplacement " + chemFichCamExp
        );
}


void Camion::afficherStatistiques()const
{
    QSqlQuery query;
    query.prepare("SELECT capacite_max FROM CAMIONS");
    if (!query.exec()) {
        qDebug() << "Erreur SQL:" << query.lastError().text();
        return;
    }

    int niveau1 = 0, niveau2 = 0, niveau3 = 0, niveau4 = 0, niveau5 = 0;
    while (query.next()) {
        double capacite = query.value(0).toDouble();
        if      (capacite >= 0 && capacite < 3) niveau1++;
        else if (capacite >= 3 && capacite < 7) niveau2++;
        else if (capacite >= 7 && capacite < 14) niveau3++;
        else if (capacite >= 14 && capacite <= 20) niveau4++;
        else if (capacite > 20) niveau5++;
    }

    QPieSeries *series = new QPieSeries();
    if (niveau1 > 0) series->append("0-3t (" + QString::number(niveau1) + ")", niveau1);
    if (niveau2 > 0) series->append("3-7t (" + QString::number(niveau2) + ")", niveau2);
    if (niveau3 > 0) series->append("7-14t (" + QString::number(niveau3) + ")", niveau3);
    if (niveau4 > 0) series->append("14-20t (" + QString::number(niveau4) + ")", niveau4);
    if (niveau5 > 0) series->append("20t (" + QString::number(niveau5) + ")", niveau5);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Statistiques des camions\nselon la capacité");
    chart->legend()->setAlignment(Qt::AlignBottom);

    QDialog dialog;
    QChartView chartView(&dialog);
    chartView.setChart(chart);
    chartView.setRenderHint(QPainter::Antialiasing);

    QVBoxLayout layout(&dialog);
    layout.addWidget(&chartView);

    for (QPieSlice *slice : series->slices()) {
        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        slice->setLabelArmLengthFactor(0.15);
    }

    dialog.resize(700, 500);
    dialog.exec();
}


void Camion::camMapZoom(QQuickWidget * mapWidget, bool i)const{
    if(!mapWidget || !mapWidget->rootObject())return;

    QObject *map = mapWidget->rootObject()->findChild<QObject*>("mainMap");
    if(map){
        double currentZoom = map->property("zoomLevel").toDouble();
        switch(i)
        {
        case false:
            if(currentZoom > 15){
                map->setProperty("zoomLevel", currentZoom - 1);
            }
            break;
        case true:
            if ( currentZoom < 20){
                map->setProperty("zoomLevel", currentZoom + 1);
            }
            break;
        }


    }
}

void Camion::camMapCentrer(QQuickWidget* mapWidget)const{
    if(!mapWidget || !mapWidget->rootObject())return;

    QObject *map = mapWidget->rootObject()->findChild<QObject*>("mainMap");
    if(map){
        QGeoCoordinate centre(36.897699, 10.189650);
        map->setProperty("center", QVariant::fromValue(centre));
        map->setProperty("zoomLevel", 17);
    }
}
void Camion::camMapShowPC(QQuickWidget * mapWidget){
    if(!mapWidget) return;

    QSqlQuery query;
    query.prepare("SELECT p.ZONE, p.ADRESSE, p.NIVEAU, "
                  "CAST(c.LATITUDE AS NUMBER), "
                  "CAST(c.LONGITUDE AS NUMBER) "
                  "FROM POINTS_DE_COLLECTE p "
                  "JOIN COORD_PC c ON p.ID_PC = c.ID_PC");

    if(!query.exec()){
        qDebug() << "Erreur SQL :" << query.lastError().text();
        return;
    }
    QMetaObject::invokeMethod(mapWidget->rootObject(), "clearMap");
    while (query.next()){
        QString zone = query.value(0).toString();
        QString adresse = query.value(1).toString();
        int niveau = query.value(2).toInt();
        double lat = query.value(3).toDouble();
        double lng = query.value(4).toDouble();

        QMetaObject::invokeMethod(mapWidget->rootObject(), "addPoint",
                                  Q_ARG(QVariant,lat),
                                  Q_ARG(QVariant,lng),
                                  Q_ARG(QVariant,zone+ " - " + adresse),
                                  Q_ARG(QVariant,niveau));
    }
}
void Camion::camConvSauvCoord( QString adresse){
    QSqlQuery queryId;
    int id;

    if(queryId.exec("SELECT MAX(ID_PC) FROM POINTS_DE_COLLECTE") && queryId.next()){
        id = queryId.value(0).toInt();
    }
    QMap<QString, QVariant> parameters;
    parameters["osm.useragent"] = "RecyTrack_Project_Student_Esprit";

    QGeoServiceProvider *service = new QGeoServiceProvider("osm", parameters);
    QGeoCodingManager *geoManager = service->geocodingManager();

    if(geoManager){
        QGeoCodeReply *reply = geoManager->geocode(adresse);

        QObject::connect(reply, &QGeoCodeReply::finished, [reply, id, adresse](){
            if(reply->error() == QGeoCodeReply::NoError){
                QSqlQuery query;
                if(!reply->locations().empty()){
                    QGeoLocation loc = reply->locations().first();
                    double lat = loc.coordinate().latitude();
                    double lng = loc.coordinate().longitude();


                    query.prepare("INSERT INTO COORD_PC (ID_PC, LATITUDE, LONGITUDE)"
                                  "VALUES (:id, :lat, :lng)");
                    query.bindValue(":id", id);
                    query.bindValue(":lat", lat);
                    query.bindValue(":lng", lng);
                }else
                {

                    query.prepare("INSERT INTO COORD_PC (ID_PC)"
                                  "VALUES (:id)");
                    query.bindValue(":id", id);
                    QMessageBox::warning(nullptr,
                                         "Probleme localisation",
                                         "L'adresse ajouté est trop vague ou introuvalbe ");
                }

                if(!query.exec())
                {
                    qDebug() << "Erreur SQL :" << query.lastError().text();
                }
            }else{
                qDebug() << "Erreur géocodage pour " << adresse << ":" << reply->errorString();
            }
            reply->deleteLater();
        });
    }
}
void Camion::camConvModifCoord(int id, QString adresse){
    QMap<QString, QVariant> parameters;
    parameters["osm.useragent"] = "RecyTrack_Project_Student_Esprit";
    QGeoServiceProvider *service = new QGeoServiceProvider("osm", parameters);
    QGeoCodingManager *geoManager = service->geocodingManager();
    if(geoManager){
        QGeoCodeReply *reply = geoManager->geocode(adresse);
        QObject::connect(reply, &QGeoCodeReply::finished, [reply,id,adresse](){
            if(reply->error() == QGeoCodeReply::NoError && !reply->locations().empty()){

                QGeoLocation loc = reply->locations().first();
                double lat = loc.coordinate().latitude();
                double lng = loc.coordinate().longitude();

                QSqlQuery query;
                query.prepare("UPDATE COORD_PC SET "
                              "LATITUDE = :lat, "
                              "LONGITUDE = :lng "
                              "WHERE ID_PC = :id");
                query.bindValue(":lat", lat);
                query.bindValue(":lng", lng);
                query.bindValue(":id", id);
                if(!query.exec())
                {
                    qDebug() << "Erreur SQL :" << query.lastError().text();
                }

            }else{
                qDebug() << "Erreur géocodage pour " << adresse << ":" << reply->errorString();
            }
        });
    }
}


