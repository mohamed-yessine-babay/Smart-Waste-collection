#ifndef ROUTEMANAGER_H
#define ROUTEMANAGER_H

#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QVector>
#include <QPair>

struct CollectPoint {
    int    id_pc;
    double lat, lng;
};

class RouteManager : public QObject {
    Q_OBJECT
public:
    explicit RouteManager(QObject* parent = nullptr);

    Q_INVOKABLE void computeRoute(int idCamion);


signals:
    void routeReady(QVariantList orderedCoords, QString polylineGeojson, double distKm, double durMin);
    void routeError(QString message);
    void routeSaved(int idItineraire);

private:
    QNetworkAccessManager* m_nam;

    QVector<CollectPoint> loadPoints();
    QVector<int> nearestNeighbor(const QVector<CollectPoint>& pts);
    void twoOpt(QVector<int>& order, const QVector<CollectPoint>& pts);
    double dist(const CollectPoint& a, const CollectPoint& b);
    void callOsrm(const QVector<CollectPoint>& fullRoute, int idCamion);
    void saveRoute(const QVector<int>& order, const QVector<CollectPoint>& pts, int idCamion, double totalKm, double totalMin);
};
#endif // ROUTEMANAGER_H
