#ifndef WASTERECYCLING_H
#define WASTERECYCLING_H

#include <QMainWindow>
#include <QGraphicsBlurEffect>
#include <QAbstractItemView>
#include "machine.h"
#include "employee.h"
#include "conteneur.h"
#include "Camion.h"
#include "routemanager.h"
#include "affectation.h"
#include "client.h"
#include "point_de_collecte.h"
#include "arduino.h"
#include "recyclage.h"
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QColor>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
//new
#include <QtCharts>
#include <QChartView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QLineSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QListWidget>
#include <QTextEdit>
#include <QJsonObject>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <QNetworkAccessManager>
#include <QScatterSeries>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QVector>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include <QPrinter>
#include <QFileDialog>
#include <QTextDocument>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>

/////////////////////////progress bar GPC niveau de remplissage ---------------------------------------------------------------
class ProgressBarDelegate : public QStyledItemDelegate
{
public:
    explicit ProgressBarDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

private:
    QColor getFillColor(int niveau) const;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class WasteRecycling;
}
QT_END_NAMESPACE

class WasteRecycling : public QMainWindow
{
    Q_OBJECT

public:
    WasteRecycling(QWidget *parent = nullptr);
    ~WasteRecycling();

private slots:

    /*/ ********************* SLOTS NAVIGATION SIDEBAR *************************
    void on_GEmpBtn_clicked();
    void on_GContBtn_clicked();
    void on_GMachinBtn_clicked();
    void on_GCamBtn_clicked();
    void on_GPcBtn_clicked();
    void on_GClientBtn_clicked();
    */// ********************* SLOTS NAVIGATION SIDEBAR *************************

    // ********************* SLOTS GESTION POINTS DE COLLECTE *************************
    void on_GPC_BtnSave_clicked();
    void on_GPC_BtnModifier_clicked();
    void on_gpcBtnSupprimer_clicked();
    void on_gpcTableView_doubleClicked(const QModelIndex &index);
    void on_GPC_TxtSearch_textChanged(const QString &text);
    void on_GPC_CmbFilter_currentTextChanged(const QString &text);// Applique le tri métier selon la priorité sélectionnée
    void on_GPC_BtnExport_clicked();
    void on_GPC_BtnPredict_clicked();// Déclenche la prédiction de saturation pour le point sélectionné

    void update_label(QByteArray data);
    void on_GPC_BtnMeasure_clicked();


    // ********************* SLOTS GESTION POINTS DE COLLECTE *************************


// ********************* SLOTS GESTION CAMION *************************

    // Navigation Metier Crud
    void on_crudMet1CamBtn_clicked();
    void on_crudMet2CamBtn_clicked();
    void on_f1AccCamBtn_clicked();
    void on_f1Met2CamBtn_clicked();
    void on_f2AccCamBtn_clicked();
    void on_f2Met1CamBtn_clicked();

    // Fonctionalite Crud
    void on_addCamBtn_clicked();
    void on_delCamBtn_clicked();
    void on_tableCam_doubleClicked(const QModelIndex &index);

    // Fonctionalite Métier
    void on_rechCamBtn_clicked();
    void camMatSearch(const QString &text);
    void on_triCamBtn_clicked();
    void on_pdfCamBtn_clicked();
    void on_statCamBtn_clicked();
    void mettreAJourLaCarte(const QList<QGeoCoordinate>& listePoints);
    void on_camCentreBtn_clicked();
    void on_camZoomBtn_clicked();
    void on_camDezoomBtn_clicked();
    void on_camMapRaf_clicked();
    void on_camItinBtn_clicked();
    void on_camAffTable_doubleClicked(const QModelIndex &index);
    void on_camAffPropBtn_clicked();
    void on_camAffAddBtn_clicked();
    void on_camShowRempBtn_clicked();
// ********************* SLOTS GESTION CAMION *************************


// ********************* SLOTS GESTION CLIENT *************************
    void on_clbtnajt_clicked();
    void on_clbtnsup_clicked();
    void on_clbtnmodif_clicked();
    void on_clbtnstat_clicked();
    void on_clbtnsysprio_clicked();
    void on_clbtnsysfide_clicked();
    void on_clretour1_clicked();
    void on_clretour2_clicked();
    void on_clretour3_clicked();
    void on_clsearch_textChanged(const QString &text);
    void on_clbtntri_clicked();
    void on_clbtnpdf_clicked();
    void on_cllistec_doubleClicked(const QModelIndex &index);
    void chargerFidelite();
    void on_clbtnvocSup_clicked();
    QString genererMessageLocal(QString nom, int points);
// ********************* SLOTS GESTION CLIENT *************************



    //--------------------------------------container------------------------------
    // CRUD Slots (Controller)
    void on_GCont_addButton_clicked();        // CREATE
    void on_GCont_refreshButton_clicked();    // READ (refresh)
    void on_GCont_updateButton_clicked();     // UPDATE
    void on_GCont_deleteButton_clicked();     // DELETE

    // UI Slots
    void on_GCont_clearButton_clicked();
    void on_GCont_previewButton_clicked();
    void on_GCont_sortButton_clicked();
    void on_GCont_searchButton_clicked();
    void on_GCont_exportPDFButton_clicked();   // PDF Export
    void on_GCont_predictionButton_clicked();  // Prediction button
    void on_GCont_statisticsButton_clicked();  // Statistics button
    void on_GCont_containerTable_doubleClicked(const QModelIndex &index);
    void on_GCont_containerTable_clicked(const QModelIndex &index);  // Single click for Arduino

    // Arduino Slots
    void onArduinoDataReceived();
    void onArduinoError(QSerialPort::SerialPortError error);
    void attemptArduinoReconnection();


    //--------------------------------------container------------------------------
    // Container Management Tab Slots (only remaining functions)

    //--------------------------------------machine------------------------------
    void on_MRbtn_lancer_clicked();
    void on_MRbtn_supprimer_clicked();
    void on_MRtable_doubleClicked(const QModelIndex &index);
    void on_MRtable_clicked(const QModelIndex &index);
    void on_MRbtn_stats_clicked();
    void on_MRline_recherche_returnPressed();
    void on_MRbtn_trier_clicked();
    void on_MRbtn_export_clicked();

    void on_MRbtn_machine_clicked();
    void on_MRbtn_recyclage_clicked();
    //--------------------------------------machine------------------------------

    //--------------------------------------recyclage------------------------------
    void on_MRbtn_lancer_2_clicked();
    void on_MRbtn_supprimer_2_clicked();
    void on_MRtable_2_clicked(const QModelIndex &index);
    void lireDonneesCapteur(QByteArray buffer);

    void on_MRbtn_machine_2_clicked();
    void on_MRbtn_recyclage_2_clicked();
    //--------------------------------------recyclage------------------------------

    //*************************************EMPLOYEE******************************************
    void refreshTable(employee e);
    void onRightClick(const QPoint &pos);
    void showEmployeeStats();
    void employeeExportToPDF();
    //*************************************EMPLOYEE******************************************



private:
    ///////////////////////////////////////////animation
    QMap<QWidget*, QRect> originalGeometries;
    QGraphicsProxyWidget *statBtnProxy;
    ///////////////////////////////////////////arduino
    Arduino A;
    QByteArray serialBuffer;

    ///////////////////////////////////////////GPC
    PointCollecte PC;
    int gpcSelectedId = -1;

    bool gpcMesureEnCours = false;
    int gpcIdMesure = -1;

    QByteArray data;

    // Retourne une couleur selon le niveau de remplissage
    QColor getFillColor(int niveau);
    ////////////////////////////////
    Ui::WasteRecycling *ui;
    enum PageGestion{
        PAGE_GEMPLOYE = 0,
        PAGE_GCONTAINER = 1,
        PAGE_GMACHINE = 2,
        PAGE_GCAMION = 3,
        PAGE_GPOINT_COLLECTE = 4,
        PAGE_GCLIENT = 5
    };
    // PAGE STACKED

///////////////////////////GCAMION
    // PAGES CAMION
    enum PagesGCamion {
        PAGE_CAMION_CRUD = 0,
        PAGE_CAMION_F1 = 2,
        PAGE_CAMION_F2 = 1
    };
    // Objet Temporaire
    Camion CamTemp;
    Affectation AffTemp;
    RouteManager* m_routeMgr;
    // fonctions supplementaires
    void resetAddEditCamForm();

//////////////////////////////////
    // PAGE CLIENT
    client            ClientTemp;
    QSqlQueryModel   *m_model = nullptr;   // modèle lié au QTableView

    enum Pages {
        PAGE_CRUDCLIENT    = 0,
        PAGE_STATCLIENT    = 1,
        PAGE_SYSFIDECLIENT = 2,
        PAGE_SYSPRIOCLIENT = 3
    };
    // Enum vocal
    enum ModeVocal { VOCAL_AUCUN, VOCAL_CHAMP, VOCAL_SUPPRESSION };
    ModeVocal  m_modeVocal  = VOCAL_AUCUN;
    QLineEdit *m_champCible = nullptr;
    QString    m_champType;

    // Helpers vocaux
    void attendreArduino(QLineEdit *champ, const QString &type = "");
    void attendreArduinoSuppression();
    void traiterAudioArduino(const QString &wavPath);
    void traiterCommandeSuppression(const QString &texte);
    void remplirChampDepuisTexte(const QString &texte);

    // Messagerie IA
    QNetworkAccessManager *m_network    = nullptr;
    QMediaCaptureSession  *m_session    = nullptr;
    QMediaRecorder        *m_recorder   = nullptr;
    QString                m_wavPath;
    QString                 m_groqKey = qEnvironmentVariable("GROQ_API_KEY"); 
    QString                m_anthropicKey = "...";

    void envoyerAudioWhisper(const QString &wavPath);
    void envoyerTexteAuLLM(const QString &texte);
    void ouvrirDialoguePreRempli(const QJsonObject &data);
    void setupMessagerieIA();
    // ── Helpers ──
    void chargerClients();                          // charge/recharge la table depuis la BDD
    int  selectedId() const;                        // retourne l'id_client de la ligne sélectionnée (-1 si aucune)
    bool isValidEmail(const QString &email);

// PAGE CLIENT
//////////////////////////////////

    //--------------------------------------machine------------------------------
    void afficherMRtab();
    void afficherStatistiquesMachines();
    int getCapaciteTotaleMachines();
    double getCapaciteMoyenneMachines();

    Machine mach;
    //--------------------------------------machine------------------------------

    //-----------------------------------recyclage------------------------------

    Recyclage recy;
    float poidsDernierRecu = 0.0;
    modifier_sortie modif;
    //-----------------------------------recyclage------------------------------

    //--------------------------------------container------------------------------
    Conteneur C;        // Objet de la classe Conteneur (Modele)

    // Helper Methods
    void refreshTable();                    // Actualiser l'affichage
    void clearForm();
    void fillFormFromConteneur(const Conteneur& conteneur);
    bool validateForm();
    Conteneur getConteneurFromForm();
    int getSelectedId();

    // PDF Generation Methods
    void generatePDFReport();
    QString getCurrentDateTime();

    // Prediction Methods
    void showPredictionDetails(const Conteneur& conteneur);

    // Statistics Method
    void showStatisticsDialog();

    // Arduino Integration Methods
    void setupArduino();                    // Setup Arduino connection
    void connectToArduino();                // Connect to Arduino
    void disconnectFromArduino();           // Disconnect from Arduino
    void sendToArduino(int id, const QString& type, int fillPercentage); // Send data to Arduino
    void updateArduinoStatus(bool connected); // Update UI status
    void scanForArduinoPort();              // Auto-scan for Arduino port

    // Arduino Members
    QSerialPort* arduinoSerial;             // Serial port for Arduino
    QTimer* arduinoReconnectTimer;          // Timer for reconnection attempts
    QLabel* arduinoStatusLabel;             // Status label in status bar
    bool arduinoConnected;                  // Connection state flag

    //--------------------------------------container------------------------------


    // GPC statistiques
    // Charge les statistiques du module Point de collecte dans l’interface
    void chargerStatistiquesGPC();

    void creerChartBarZone();
    void creerChartPiePriorite();
    void animerPageStatistiqueGPC();
    //Nettoyer un layout avant de recréer un graphe ou recharger un contenu
    void nettoyerLayout(QWidget *widget);

    // Charge et affiche le graphique de classification d’urgence par zone
    void chargerClassificationGPC();

    void resetPredictionFields();
    void chargerTableClassificationGPC();
    //animation methods
    void animateMove(QWidget *widget, QPoint endPos, int duration = 300);
    void animateFade(QWidget *widget, float startOpacity, float endOpacity, int duration = 300);
    void animateGrow(QWidget *widget, int amount = 5, int duration = 150);
    void animateShrink(QWidget *widget, int amount = 5, int duration = 150);
    void animateRotate(QGraphicsProxyWidget *proxy, int startAngle, int endAngle, int duration);
    void animatePageTransition(QStackedWidget *stackedWidget, int newIndex);
    void animatePageTransitionVertical(QStackedWidget *stackedWidget, int newIndex);
    void updateSidebarStyles();

protected :
    // recyclage clique dans le vide
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};
#endif // WASTERECYCLING_H
