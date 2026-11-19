#ifndef AFFECTATION_H
#define AFFECTATION_H
#include "Camion.h"
#include "employee.h"

class Affectation{
private:
    bool propose = false;
public:
    Camion Cam;
    employee emp;
    bool affecterCamionEmployee();
    QSqlQueryModel * affectationShow();
    bool affectationDelete();
    bool affCheckFreeEmployee();
    bool affCheckFreeCamion();
    bool affGetIdEmpByFullName(QString nom,QString prenom);
    bool affProposerAffectation();
    bool fetchAffectPK(QString nom, QString prenom, QString matricule);
    bool affFetchCamion(QString matricule);
    bool affFreeCamion();
    bool affUseCamion();

};

#endif // AFFECTATION_H
