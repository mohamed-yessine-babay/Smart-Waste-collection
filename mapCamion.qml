import QtQuick
import QtLocation
import QtPositioning

Rectangle {
    id: root
    anchors.fill: parent

    property var routePath: []
    property bool routeVisible: false

    // --- 1. LE MODÈLE (Déclaré en haut pour être global) ---
    ListModel {id: markerModel}
    ListModel {id: routeStopsModel }
    ListModel {id: arrowModel}

    // --- 2. LE PLUGIN (Configuration anti-watermark) ---
    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter { name: "osm.useragent"; value: "RecyTrack_Final_App" }
        PluginParameter { name: "osm.mapping.providersvisible"; value: false }
        PluginParameter { name: "osm.mapping.custom.host"; value: "https://tile.openstreetmap.org/" }
    }

    // --- 3. LA CARTE ---
    Map {
        id: mainMap
        objectName: "mainMap"
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(36.897699, 10.189650)
        zoomLevel: 17

        // Forcer le mode personnalisé pour éviter les serveurs morts de Qt
        activeMapType: supportedMapTypes[supportedMapTypes.length - 1]

        // 2. Limiter le zoom pour ne pas voir toute la planète
        minimumZoomLevel: 15
        maximumZoomLevel: 20


        property var tunisiaBounds: QtPositioning.rectangle(
                QtPositioning.coordinate(37.6, 7.5), // Nord-Ouest
                QtPositioning.coordinate(30.0, 11.6)  // Sud-Est
            )

        // 4. Forcer le maintien dans la zone
        onCenterChanged: {
            // Si le centre de la carte sort de la zone Tunisie, on le remet à l'intérieur
            if (!tunisiaBounds.contains(center)) {
                center = QtPositioning.coordinate(36.897699, 10.189650)
            }
        }
        MouseArea {
            anchors.fill: parent
            onWheel: (wheel) => {
                if (wheel.angleDelta.y > 0) {
                    if (mainMap.zoomLevel < mainMap.maximumZoomLevel) mainMap.zoomLevel += 1
                } else {
                    if (mainMap.zoomLevel > mainMap.minimumZoomLevel) mainMap.zoomLevel -= 1
                }
            }
            cursorShape: containsPress ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            property real lastX: 0
            property real lastY: 0
            onPressed: (mouse) => { lastX = mouse.x; lastY = mouse.y }
            onPositionChanged: (mouse) => {
                if (mouse.buttons & Qt.LeftButton) {
                    mainMap.pan(lastX - mouse.x, lastY - mouse.y)
                    lastX = mouse.x; lastY = mouse.y
                }
            }
        }

        MapItemView {
            model: markerModel
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(model.lat, model.lng)
                anchorPoint.x: 10; anchorPoint.y: 10
                sourceItem: Rectangle {
                    width: 20; height: 20; radius: 10;
                    border.color: "white"
                    border.width: 2

                    color: {
                        if(model.level >= 0 && model.level <= 49)return "green";
                        if(model.level >= 50 && model.level <= 79)return "orange";
                        if(model.level >= 80 && model.level <= 100)return "red";
                        return "gray";
                    }
                }
            }
        }

        MapPolyline {
            id: routePolyline
            line.width: 4
            line.color: "#2196F3"
            visible: root.routeVisible   // ← lié à la property racine
            path: root.routePath
        }


        MapItemView {
            model: arrowModel
            delegate: MapQuickItem{
                coordinate: QtPositioning.coordinate(model.lat, model.lng)
                anchorPoint.x: 8; anchorPoint.y: 8
                sourceItem: Canvas {
                    width: 16; height: 16
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0,0,16,16)
                        ctx.save()
                        ctx.translate(8,8)
                        ctx.rotate(model.angle * Math.PI/180)
                        ctx.beginPath()
                        ctx.moveTo(0,-7)
                        ctx.lineTo(5, 4)
                        ctx.lineTo(-5,4)
                        ctx.closePath()
                        ctx.fillStyle = "#2196F3"
                        ctx.fill()
                        ctx.restore()
                    }
                }
            }
        }

        MapItemView {
            model: routeStopsModel
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(model.lat, model.lng)
                anchorPoint.x: 16; anchorPoint.y: 16
                sourceItem: Rectangle {
                    width: 32; height: 32; radius: 16
                    color: "#2196F3"
                    border.color: "white"; border.width: 2
                    Text {
                        anchors.centerIn: parent
                        text: model.ordre
                        color: "white"; font.bold: true; font.pixelSize: 13
                    }
                }
            }
        }
    }

    // --- 4. FONCTIONS DE COMMUNICATION (Accessibles par C++) ---
    function clearMap() {
        markerModel.clear();
    }

    function addPoint(lat, lng, name, level) { // <--- Ajoutez "name" ici
        markerModel.append({
            "lat": lat,
            "lng": lng,
            "name": name,
            "level": level
        });
    }

    function drawRoute(geojson, stops) {
        // Trace la polyline
        var geo = JSON.parse(geojson)
        var path = []
        for (var i = 0; i < geo.coordinates.length; i++) {
            path.push(QtPositioning.coordinate(
                geo.coordinates[i][1],  // lat
                geo.coordinates[i][0]   // lng
            ))
        }
        root.routePath    = path    // ← accès direct à la property
        root.routeVisible = true

        // Remplit les marqueurs numérotés
        routeStopsModel.clear()
        for (var j = 0; j < stops.length; j++) {
            routeStopsModel.append({
                "lat":   stops[j].lat,
                "lng":   stops[j].lng,
                "ordre": j + 1
            })
        }

        arrowModel.clear()
        var step = Math.max(1,Math.floor(geo.coordinates.length / 10))
        for(var k = step; k < geo.coordinates.length - 1; k+=step){
            var p1 = geo.coordinates[k]
            var p2 = geo.coordinates[k + 1]
            var dlng = p2[0] - p1[0]
            var dlat = p2[0] - p1[0]
            var angle = Math.atan(dlng, dlat) * 180 / Math.PI
            arrowModel.append({
                              "lat" : (p1[1] + p2[1]) / 2,
                              "lng" : (p1[0] + p2[0]) / 2,
                              "angle" : angle
            })
        }
    }

    function clearRoute() {
        routePolyline.path = []
        routePolyline.visible = false
        routeStopsModel.clear()
        arrowModel.clear()
    }
}
