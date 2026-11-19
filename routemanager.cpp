#include "routemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtMath>
#include <QDebug>
#include <numeric>

RouteManager::RouteManager(QObject* parent)
    : QObject(parent), m_nam(new QNetworkAccessManager(this)) {}

// --- Charge tous les points depuis COORDONNEES_PC ---
QVector<CollectPoint> RouteManager::loadPoints() {
    QVector<CollectPoint> pts;
    QSqlQuery q;
    q.prepare("SELECT id_pc, "
              "TO_CHAR(latitude, '99.999999'), "
              "TO_CHAR(longitude, '99.999999') "
              "FROM COORD_PC");
    if (!q.exec()) {
        emit routeError("Erreur chargement points: " + q.lastError().text());
        return pts;
    }
    while (q.next())
        pts.append({ q.value(0).toInt(),
                    q.value(1).toDouble(),
                    q.value(2).toDouble() });
    return pts;
}

// --- Distance euclidienne approchée (suffisant pour le tri) ---
double RouteManager::dist(const CollectPoint& a, const CollectPoint& b) {
    double dlat = a.lat - b.lat;
    double dlng = (a.lng - b.lng) * qCos(qDegreesToRadians(a.lat));
    return qSqrt(dlat*dlat + dlng*dlng);
}

// --- Nearest Neighbor ---
QVector<int> RouteManager::nearestNeighbor(const QVector<CollectPoint>& pts) {
    int n = pts.size();
    QVector<bool> visited(n, false);
    QVector<int>  order;
    order.reserve(n);

    int current = 0; // on part du premier point (peut être le dépôt)
    visited[0] = true;
    order.append(0);

    for (int step = 1; step < n; ++step) {
        double bestDist = 1e18;
        int    bestIdx  = -1;
        for (int j = 0; j < n; ++j) {
            if (!visited[j]) {
                double d = dist(pts[current], pts[j]);
                if (d < bestDist) { bestDist = d; bestIdx = j; }
            }
        }
        visited[bestIdx] = true;
        order.append(bestIdx);
        current = bestIdx;
    }
    return order;
}

// --- 2-opt ---
void RouteManager::twoOpt(QVector<int>& order, const QVector<CollectPoint>& pts) {
    int n = order.size();
    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 2; j < n; ++j) {
                // distance avant inversion : i→i+1  et  j→j+1
                double before = dist(pts[order[i]], pts[order[i+1]])
                                + dist(pts[order[j]], pts[order[(j+1)%n]]);
                // distance après inversion : i→j  et  i+1→j+1
                double after  = dist(pts[order[i]], pts[order[j]])
                               + dist(pts[order[i+1]], pts[order[(j+1)%n]]);
                if (after < before - 1e-9) {
                    // on inverse le segment [i+1 .. j]
                    std::reverse(order.begin()+i+1, order.begin()+j+1);
                    improved = true;
                }
            }
        }
    }
}

// --- Point d'entrée principal ---
void RouteManager::computeRoute(int idCamion) {
    QVector<CollectPoint> pts = loadPoints();
    if (pts.size() <2) return;

    CollectPoint depot = pts.first();

    QVector<CollectPoint> stops = pts.mid(1);
    QVector<int> order = nearestNeighbor(stops);
    twoOpt(order, stops);

    QVector<CollectPoint> fullRoute;
    for (int idx : order)
        fullRoute.append(stops[idx]);
    fullRoute.append(depot);

    callOsrm(fullRoute, idCamion);
}

// --- Appel OSRM ---
void RouteManager::callOsrm(const QVector<CollectPoint>& fullRoute,int idCamion)
{
    QString coords;
    for (int i = 0; i < fullRoute.size(); ++i) {
        if (i > 0) coords += ";";
        coords += QString::number(fullRoute[i].lng, 'f', 6) + "," +
                  QString::number(fullRoute[i].lat, 'f', 6);
    }

    QString url = "https://router.project-osrm.org/route/v1/driving/"
                  + coords + "?overview=full&geometries=geojson&steps=false";

    QNetworkRequest req(url);
    QNetworkReply* reply = m_nam->get(req);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit routeError("OSRM error: " + reply->errorString());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject route = doc["routes"].toArray().first().toObject();
        double distKm  = route["distance"].toDouble() / 1000.0;
        double durMin  = route["duration"].toDouble() / 60.0;
        QString geojson = QString(QJsonDocument(route["geometry"].toObject()).toJson(QJsonDocument::Compact));


        QVector<CollectPoint> stopOnly = fullRoute.mid(1, fullRoute.size() - 2);
        QVector<int> orderedIdx(stopOnly.size());
        std::iota(orderedIdx.begin(), orderedIdx.end(),0);
        saveRoute(orderedIdx, stopOnly, idCamion, distKm, durMin);

        QVariantList coords;
        for (int i = 0; i< fullRoute.size(); ++i)
            coords.append(QVariantMap{{"lat", fullRoute[i].lat},
                                      {"lng", fullRoute[i].lng},
                                      {"id",  fullRoute[i].id_pc},
                                      {"ordre", i}});

        routeReady(coords, geojson, distKm, durMin);
    });
}

// --- Sauvegarde en base ---
void RouteManager::saveRoute(const QVector<int>& order,
                             const QVector<CollectPoint>& pts,
                             int idCamion,
                             double totalKm, double totalMin) {
    QSqlQuery q;
    q.prepare("INSERT INTO ITINERAIRE (id_camion, distance_km, duree_min) "
              "VALUES (:cam, :km, :min)"
              "RETURNING id_itineraire INTO :id");
    q.bindValue(":cam", idCamion);
    q.bindValue(":km",  totalKm);
    q.bindValue(":min", totalMin);
    q.bindValue(":id", 0, QSql::Out);


    if (!q.exec()) {
        qWarning() << "Erreur INSERT ITINERAIRE:" << q.lastError().text();
        return;
    }

    int idItin = q.boundValue(":id").toInt();
    qDebug() << "id_itineraire généré:" << idItin;

    QSqlQuery commitQ;
    commitQ.exec("COMMIT");

    if (idItin <= 0) {
        qWarning() << "id_itineraire invalide, rollback";
        return;
    }
    for (int step = 0; step < order.size(); ++step) {
        QSqlQuery eq;  // ← même db
        eq.prepare("INSERT INTO ETAPE_ITINERAIRE "
                   "(id_itineraire, id_pc, ordre_visite) "
                   "VALUES (:itin, :pc, :ord)");
        eq.bindValue(":itin", idItin);
        eq.bindValue(":pc",   pts[order[step]].id_pc);
        eq.bindValue(":ord",  step + 1);

        if (!eq.exec()) {
            qWarning() << "Erreur INSERT ETAPE:" << eq.lastError().text();
            return;
        }
        qDebug() << "Étape" << step+1 << "OK, id_pc=" << pts[order[step]].id_pc;
    }
}
