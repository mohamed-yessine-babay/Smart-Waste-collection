#include "affectation.h"

bool Affectation::affecterCamionEmployee()
{
    bool ins;
    QSqlQuery query;
    query.prepare("INSERT INTO CONDUIRE (ID_EMPLOYEE, ID_CAMION) VALUES (:id_emp,:id_cam)");
    query.bindValue(":id_emp", emp.get_id());
    query.bindValue(":id_cam", Cam.getId_camion());
    return query.exec();
}
bool Affectation::affGetIdEmpByFullName(QString nom,QString prenom){
    QSqlQuery query;
    query.prepare("select id_employee from employee where(nom = :nom AND prenom = :prenom AND poste = 'Chauffeur')");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom",prenom);
    if(query.exec() && query.next())
    {

        this->emp.set_id(query.value(0).toInt());
        return true;
    }
    return false;
}

bool Affectation::affCheckFreeEmployee()
{
    QSqlQuery query;
    query.prepare("SELECT ID_EMPLOYEE FROM CONDUIRE WHERE ID_EMPLOYEE = :id");
    query.bindValue(":id", emp.get_id());
    if(query.exec())
    {
        if(!query.next())
            return true;
        return false;
    }
    qDebug() << "Erreur SQL: " << query.lastError().text();
    return false;
}

bool Affectation::affCheckFreeCamion()
{
    QSqlQuery query;
    query.prepare("SELECT ID_CAMION FROM CAMIONS "
                  "WHERE STATUS = 'Disponible' "
                  "AND ID_CAMION = :id");
    query.bindValue(":id",Cam.getId_camion());
    if(query.exec())
    {
        if(query.next())
            return true;
        return false;
    }
    qDebug() << "Erreur SQL: " << query.lastError().text();
    return false;
}


bool Affectation::affProposerAffectation()
{
    QSqlQuery queryEmp;
    queryEmp.prepare("SELECT id_employee, nom, prenom "
                     "FROM EMPLOYEE "
                     "WHERE poste = 'Chauffeur' "
                     "AND id_employee NOT IN (SELECT id_employee FROM CONDUIRE)");
    if(queryEmp.exec() && queryEmp.next())
    {
        emp.set_id(queryEmp.value(0).toInt());
        emp.set_nom(queryEmp.value(1).toString());
        emp.set_prenom(queryEmp.value(2).toString());
        QSqlQuery queryCam;
        queryCam.prepare("SELECT id_camion, matricule from CAMIONS "
                         "where status = 'Disponible'");
        if(queryCam.exec() && queryCam.next())
        {
            Cam.setId_camion(queryCam.value(0).toInt());
            Cam.setMatricule(queryCam.value(1).toString());
            return true;
        }
        else{
            qDebug() << "Erreur SQL CAMIONS: " << queryCam.lastError().text();
            return false;
        }
    }else
    {
        qDebug() << "Erreur SQL EMPLOYEE: " << queryEmp.lastError().text();
        return false;
    }
}

QSqlQueryModel * Affectation::affectationShow(){
    QSqlQueryModel * model= new QSqlQueryModel();
    model->setQuery("select c.matricule,e.nom,e.prenom,a.date_affectation "
                    "from CONDUIRE a "
                    "JOIN EMPLOYEE e ON e.id_employee = a.id_employee "
                    "JOIN CAMIONS c ON c.id_camion = a.id_camion");
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("Matricule"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Nom Chauffeur"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Prenom Chauffeur"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Date d'affectation"));
    return model;
}

bool Affectation::fetchAffectPK(QString nom, QString prenom, QString matricule){
    if(Affectation::affGetIdEmpByFullName(nom,prenom) && Affectation::affFetchCamion(matricule)){
        return true;
    }
    return false;
}

bool Affectation::affectationDelete(){
    QSqlQuery query;
    query.prepare("DELETE from conduire "
                  "where id_camion = :id_camion "
                  "and id_employee = :id_employee");
    query.bindValue(":id_camion", Cam.getId_camion());
    query.bindValue(":id_employee", emp.get_id());
    return query.exec();
}
bool Affectation::affFetchCamion(QString matricule){
    bool test = Cam.fetchCamionByMatricule(matricule);
    return test;
}

bool Affectation::affFreeCamion(){
    QSqlQuery query;
    query.prepare("update camions set "
                  "status = 'Disponible' "
                  "where id_camion = :id");
    query.bindValue(":id",Cam.getId_camion());
    if(!query.exec()){
        qDebug() << "Erreur SQL: " << query.lastError().text();
        return false;
    }
    return true;
}
bool Affectation::affUseCamion(){
    QSqlQuery query;
    query.prepare("update camions set "
                  "status = 'Occupé' "
                  "where id_camion = :id");
    query.bindValue(":id",Cam.getId_camion());
    if(!query.exec()){
        qDebug() << "Erreur SQL: " << query.lastError().text();
        return false;
    }
    return true;
}

