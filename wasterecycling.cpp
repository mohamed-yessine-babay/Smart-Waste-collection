#include <QDateTime>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "connection.h"
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QStandardPaths>
#include <QProgressDialog>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QProgressBar>
#include <cmath>
#include "wasterecycling.h"
#include "ui_wasterecycling.h"
#include <QMessageBox>
#include <QGraphicsBlurEffect>
#include <QAbstractItemView>
#include "machine.h"
#include "employee.h"
#include "point_de_collecte.h"
#include "client.h"
#include "arduino.h"
#include "recyclage.h"
//***************************************GPC
#include <QHeaderView>
#include <QProgressBar>
#include <QTableWidgetItem>
#include <QSortFilterProxyModel>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QFormLayout>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QPageLayout>
#include <QDate>
//Stat
#include <QVBoxLayout>
#include <QPainter>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QSqlError>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QAudioInput>
#include <QMediaFormat>
#include <QSqlQuery>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QLayoutItem>
#include <QPdfWriter>
#include <QPageSize>
#include <QStandardPaths>
#include <QDateTime>
// camion
#include <QVariant>
#include <QQmlContext>
//
#include <QDebug>

#include <QtCharts/QtCharts>
#include <QPushButton>
#include <QTimeLine>
// ─── client ────────────────────────────────────────────────
// ─── Validators réutilisables ─────────────────────────────────────────────────
static const QRegularExpression RE_NOM  (R"(^[a-zA-ZÀ-ÿ\s\-']+$)");
static const QRegularExpression RE_TEL  (R"(\d{8,15})");
static const QRegularExpression RE_EMAIL(R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)");
// ─── client ────────────────────────────────────────────────
//****************************************GPC


WasteRecycling::WasteRecycling(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WasteRecycling)
{
    ui->setupUi(this);

    // conteneur

    // Initialize Arduino members




    /// *********** affichage Table Camion ******************



    ui->tableCam->setModel(CamTemp.showCamion());
    ui->camAffTable->setModel(AffTemp.affectationShow());
    ui->tableCam->setMouseTracking(true);
    ui->tableCam->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableCam->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->camAffTable->setMouseTracking(true);
    ui->camAffTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->camAffTable->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->tableCam, &QTableView::entered, this, [=](const QModelIndex& index) {
        ui->tableCam->selectRow(index.row());
    });
    /// *********** affichage Table Camion ******************


    /// gestion du signal changement de texte pour la recherche
    connect(ui->rechCamInp, &QLineEdit::textChanged, this, &WasteRecycling::camMatSearch);


    /// Initiation du calcul d'itineraire
    QQmlContext* context = ui->camMapShow->rootContext();
    m_routeMgr = new RouteManager(this);
    context->setContextProperty("routeManager", m_routeMgr);

    // Définir la source (le chemin doit commencer par qrc:/)
    ui->camMapShow->setSource(QUrl("qrc:/mapCamion.qml"));


    // Forcer la carte à remplir tout l'espace du widget
    ui->camMapShow->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // Vérification d'erreur
    if (ui->camMapShow->status() == QQuickWidget::Error) {
        qDebug() << "ERREUR QML :" << ui->camMapShow->errors();
    }


    connect(m_routeMgr, &RouteManager::routeReady, this,
            [=](QVariantList coords, QString geojson, double distKm, double durMin) {

                ui->camDistOut->setText(QString::number(distKm,'f',2) + " km");
                ui->camDurOut->setText(QString::number(durMin,'f',1) +" min");


                QObject* root = ui->camMapShow->rootObject();
                QMetaObject::invokeMethod(root, "drawRoute",
                                          Q_ARG(QVariant,geojson),
                                          Q_ARG(QVariant,QVariant::fromValue(coords)));
            });


    connect(m_routeMgr, &RouteManager::routeError, this,
            [=](QString msg){
                qWarning() << "Erreur itinéraire: " << msg;
            });

    CamTemp.camMapShowPC(ui->camMapShow);
    /// *****************************************************
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////GPC
    // Affichage initial GPC
    ui->gpcTableView->setModel(PC.afficher());
    ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));

    ui->gpcTableView->horizontalHeader()->setStretchLastSection(true);
    ui->gpcTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->gpcTableView->verticalHeader()->setVisible(false);

    ui->gpcTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->gpcTableView->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->gpcTableView->setAlternatingRowColors(true);
    ui->gpcTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->gpcTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->gpcTableView->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    ui->gpcTableView->setShowGrid(true);
    ui->gpcTableView->setWordWrap(false);

    ui->gpcTableView->verticalHeader()->setDefaultSectionSize(35);
    ui->gpcTableView->horizontalHeader()->setMinimumHeight(40);
    ui->gpcTableView->setStyleSheet(
        "QTableView {"
        "alternate-background-color: #2e5f5a;"
        "background-color: #2b6f68;"
        "color: white;"
        "gridline-color: #245c56;"
        "border: none;"
        "selection-background-color: #8bbf6a;"
        "selection-color: black;"
        "font-size: 14px;"
        "}"
        "QHeaderView::section {"
        "background-color: #3b6f7a;"
        "color: white;"
        "padding: 8px;"
        "border: none;"
        "font-weight: bold;"
        "font-size: 15px;"
        "}"
            );
        //***************************************************************client

        // Configuration du QTableView
        ui->cllistec->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->cllistec->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->cllistec->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->cllistec->horizontalHeader()->setStretchLastSection(true);
        ui->cllistec->verticalHeader()->setVisible(false);

        // Placeholder recherche
        ui->clsearch->setPlaceholderText("Rechercher dans tous les champs...");

        // Chargement initial
        chargerClients();

        // Double-clic → suppression
        connect(ui->cllistec, &QTableView::doubleClicked,
                this, &WasteRecycling::on_cllistec_doubleClicked);
    setupMessagerieIA();
    m_network = new QNetworkAccessManager(this);
    m_wavPath = QDir::tempPath() + "/recytrack_record.wav";
    //***************************************************************client
    //***********************************************

    // NAVIGATION ENTRE LES PAGES
    // ===============================


    // Aller à la page Statistiques
    connect(ui->GPC_BtnStats, &QPushButton::clicked, this, [=]() {
        chargerStatistiquesGPC();
        ui->GPC_Pages->setCurrentIndex(1);
        animerPageStatistiqueGPC();
    });;

    // Retour vers le tableau
    connect(ui->GPC_BtnSave, &QPushButton::clicked, this, [=]() {
        ui->GPC_Pages->setCurrentIndex(0);
    });

    connect(ui->GPC_BtnBackStats, &QPushButton::clicked, this, [=]() {
        ui->GPC_Pages->setCurrentIndex(0);
    });
//conflict cause of the ui

    ui->GPC_BtnPredict->setEnabled(false);

    // Aller à la page Classification
    connect(ui->GPC_BtnClassification, &QPushButton::clicked, this, [=]() {
        qDebug() << "Bouton classification clique";
        chargerClassificationGPC();
        chargerTableClassificationGPC();
        ui->GPC_Pages->setCurrentIndex(2);

    });

    // Retour vers la page principale
    connect(ui->GPC_BtnBackClass, &QPushButton::clicked, this, [=]() {
        ui->GPC_Pages->setCurrentIndex(0);
    });


    //arduinoGPC
    int ret=A.connect_arduino();//lancer la connexion a arduino
    switch (ret) {

        break;
    case (1):qDebug() << "arduino is available but not connected to : " << A.getarduino_port_name();

        break;
    case (-1):qDebug() << "arduino is not available ";

        break;

    case (0):qDebug() << "arduino is available and connected to : " << A.getarduino_port_name();

    }
    if (ret == 0) {
        QObject::connect(A.getserial(), &QSerialPort::readyRead, this, [=](){
            A.identifyArduino();
        });
    }
    QObject::connect(&A, &Arduino::gPcArd, this, &WasteRecycling::update_label);//permet de lancer
    //le slot update_label suite a la reception du signal readyread (reception des donnees)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////GPC


    //--------------machine constructeurs------------------------
    connect(ui->MRtable, &QTableView::clicked,
            this,        &WasteRecycling::on_MRtable_clicked);
    ui->MRtable->setModel( mach.afficher() );
    ui->MRtable->setColumnWidth(4, 150);
    ui->MRtable->setCornerButtonEnabled(false);
    ui->MRtable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->MRtable->setSelectionMode(QAbstractItemView::SingleSelection);

    disconnect(ui->MRbtn_stats, nullptr, nullptr, nullptr);
    connect(ui->MRbtn_stats, &QPushButton::clicked, this, &WasteRecycling::on_MRbtn_stats_clicked);

    connect(ui->MRline_recherche, &QLineEdit::returnPressed, [this]() {
        QString texte = ui->MRline_recherche->text().trimmed();
        if (texte.isEmpty()) {
            ui->MRtable->setModel(mach.afficher());
        } else {
            ui->MRtable->setModel(mach.rechercher(texte));
        }
    });

    //--------------machine constructeurs-------------------------------

    //-------------------------------Recyclage constructeurs-------------------------------
    QIntValidator *forceNombre = new QIntValidator(1, 999, this);
    ui->MRlineEdit_IdCont->setValidator(forceNombre);
    ui->MRlineEdit_IdMach->setValidator(forceNombre);
    ui->MRtable_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->MRtable_2->setModel(recy.afficherHistorique());
    ui->MRtable_2->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->MRlineEdit_IdCont, &QLineEdit::textChanged, [this](){
        ui->MRbtn_lancer_2->setText("Lancer");
        ui->MRlabel_QteSortie->setText("Nulle");
    });

    connect(&A, &Arduino::gMachArd, this, &WasteRecycling::lireDonneesCapteur);
    //-------------------------------Recyclage constructeurs-------------------------------

    //----------------------------------------------------------------------container------------------------------
    setWindowTitle("Systeme de Gestion des Conteneurs - Waste Manager Pro");
    setMinimumSize(1200, 800);

    // Initial display - READ operation
    refreshTable();
    //---------------------------------------------------------------------container------------------------------


    //**********************************ARDUINO****************************************************




    // ********************************* GESTION EMPLOYEE *****************************************

    ui->loginStackedWidget->setAttribute(Qt::WA_StyledBackground, true);

    // Create the blur effect
    /*QGraphicsBlurEffect *blur = new QGraphicsBlurEffect();

    // Set the blur radius (adjust for stronger/weaker blur)
    blur->setBlurRadius(15);

    // Apply the effect to the frame
    ui->employeeLoginFrame->setGraphicsEffect(blur);*/
    ui->employeeComboBox->view()->setCursor(Qt::PointingHandCursor);
    QGraphicsDropShadowEffect *frameshadow = new QGraphicsDropShadowEffect;
    frameshadow->setBlurRadius(15);
    frameshadow->setOffset(0, 4);
    frameshadow->setColor(QColor(0,0,0,100));
    QGraphicsDropShadowEffect *btnshadow = new QGraphicsDropShadowEffect;
    btnshadow->setBlurRadius(20);
    btnshadow->setOffset(0, 4);
    btnshadow->setColor(QColor(0,0,0,150));
    ui->employeeBtnFrame->setGraphicsEffect(frameshadow); // or whatever fits your image
    ui->empLoginReturnBtn->setGraphicsEffect(btnshadow); // or whatever fits your image
    ui->questionComboBox->view()->setMinimumWidth(450);


    // return to login page
    connect(ui->empLoginReturnBtn, &QPushButton::clicked, this, [=]() {
        ui->loginStackedWidget->setCurrentWidget(ui->loginPage); // switch page
    });
    // go to stats page
    connect(ui->employeeStatBtn, &QPushButton::clicked, this, [=]() {
        //ui->employeeStackedWidget->setCurrentWidget(ui->statsPage); // switch page
        animatePageTransition(ui->employeeStackedWidget, 1);
    });
    //return to the employee page
    connect(ui->employeeReturnBtn, &QPushButton::clicked, this, [=]() {
        //ui->employeeStackedWidget->setCurrentWidget(ui->employeePage); // switch page
        animatePageTransition(ui->employeeStackedWidget, 0);
    });
    connect(ui->logoutBtn, &QPushButton::clicked, this, [=]() {
        //ui->gestionsPages->hide();           // show employee stacked widget
        //ui->loginStackedWidget->show();               // hide login
        animateMove(ui->loginStackedWidget,QPoint(0,0), 1);
        ui->loginStackedWidget->setCurrentWidget(ui->loginPage); // show employeePage
    });
    //----------------------------------CRUD+STATS-------------------------------------------------------
    /*// ******************************** NAVIGATION SIDEBAR ***************************************


void WasteRecycling::on_GEmpBtn_clicked()
{
    ui->gestionsPages->setCurrentIndex(PAGE_GEMPLOYE);
}


void WasteRecycling::on_GContBtn_clicked()
{
    ui->gestionsPages->setCurrentIndex(PAGE_GCONTAINER);
}


void WasteRecycling::on__clicked()
{
    ui->gestionsPages->setCurrentIndex(PAGE_GMACHINE);
}


void WasteRecycling::on_GCamBtn_clicked()
{
    ui->gestionsPages->setCurrentIndex(PAGE_GCAMION);

}


void WasteRecycling::on_GPcBtn_clicked()
{
    ui->gestionsPages->setCurrentIndex(PAGE_GPOINT_COLLECTE);
}


void WasteRecycling::on_GClientBtn_clicked()
{
    ui->gestionsPages->setCurrentIndex(PAGE_GCLIENT);
}

*/// *************************** FIN NAVIGATION SIDEBAR ************************************

    employee e;
    connect(&A, &Arduino::gEmpArd, this, [&](QByteArray serialBuffer) {
        // Append incoming data to your buffer
        // Check for the newline sent by Arduino (println)
        QString cardID = QString::fromUtf8(serialBuffer).trimmed();
        if (ui->gestionsPages->currentIndex() == 0 && !ui->loginStackedWidget->isVisible()) {
            if (!cardID.isEmpty()) {
                qDebug() << "Card detected:" << cardID;
                // Now you can compare this cardID to your database or perform an action
                // if(cardID == "A1B2C3D4") { ... }
            }
            ui->RFIDLabel->setText(cardID);
        }else if (ui->loginStackedWidget->isVisible()) {
            // The whole stack is visible
            vector<QString> rf_info;
            if (!cardID.isEmpty()) {
                qDebug() << "Card detected:" << cardID;
                rf_info=e.obtain_info_by_RFID(cardID);
                if(rf_info[0] == "false" && rf_info[1] == "false"){
                    QMessageBox::warning(nullptr, "Erreur", "cette carte n'exsiste pas!");
                }else{
                    ui->empUsernameLineEdit->setText(rf_info[0]);
                    ui->empPasswordLineEdit->setText(rf_info[1]);
                    ui->loginBtn->click();
                }
            }

        }

        // Clear the buffer for the next scan
        serialBuffer.clear();

            // Now you can do whatever you want with cardID!
    });
    connect(ui->empTableView, &QTableView::customContextMenuRequested, this, &WasteRecycling::onRightClick);
    // go to password page
    connect(ui->empPasswordBtn, &QPushButton::clicked, this, [&]() {
        QString question,username;
        username=ui->empUsernameLineEdit->text();
        question=e.obtainQuestion(username);
        if(question!="false"){
            ui->loginStackedWidget->setCurrentWidget(ui->passwordPage); // switch page
            ui->empQuestionLabel->setText(question);
        }
    });
    connect(ui->empObtenirBtn, &QPushButton::clicked, this, [&]() {
        QString reponce,username,password;
        username=ui->empUsernameLineEdit->text();
        reponce=ui->employeeAnwserLineEdit->text();
        password=e.motDePasseOublier(username,reponce);
        if(password!="false"){
            ui->empPasswordLabel->setText(password);
        }
    });

    //login
    connect(ui->loginBtn, &QPushButton::clicked, this, [=]() {
        QString username, password;
        username=ui->empUsernameLineEdit->text();
        password=ui->empPasswordLineEdit->text();
        if(e.login(username,password)){
            ui->AppUsername->setText(username);
            animateMove(ui->loginStackedWidget, ui->loginStackedWidget->pos()+QPoint(0,-802), 1000);
            //ui->loginStackedWidget->hide();
            //ui->gestionsPages->show();        // hide login
            //ui->employeeStackedWidget->show();           // show employee stacked widget
            ui->gestionsPages->setCurrentIndex(PAGE_GPOINT_COLLECTE); // show PgcPage
            if(e.accessRights(username)=="Administrateur" || e.accessRights(username)=="Chef d’équipe"){
                ui->lockIconLable_1->hide();
                ui->lockIconLable_2->hide();
                ui->lockIconLable_3->hide();
                ui->lockIconLable_4->hide();
                ui->lockIconLable_5->hide();
                ui->lockIconLable_6->hide();
            }else if(e.accessRights(username)=="Chauffeur"){
                ui->lockIconLable_1->show();
                ui->lockIconLable_2->show();
                ui->lockIconLable_3->show();
                ui->lockIconLable_4->hide();
                ui->lockIconLable_5->hide();
                ui->lockIconLable_6->show();
            }else if(e.accessRights(username)=="Responsable RH"){
                ui->lockIconLable_1->hide();
                ui->lockIconLable_2->show();
                ui->lockIconLable_3->show();
                ui->lockIconLable_4->show();
                ui->lockIconLable_5->hide();
                ui->lockIconLable_6->hide();
            }else if(e.accessRights(username)=="Agent de collecte"){
                ui->lockIconLable_1->show();
                ui->lockIconLable_2->hide();
                ui->lockIconLable_3->show();
                ui->lockIconLable_4->hide();
                ui->lockIconLable_5->hide();
                ui->lockIconLable_6->show();
            }else{
                ui->lockIconLable_1->show();
                ui->lockIconLable_2->hide();
                ui->lockIconLable_3->hide();
                ui->lockIconLable_4->hide();
                ui->lockIconLable_5->hide();
                ui->lockIconLable_6->show();
            }
        }
    });
    //------------------------------ NABIGATION SIDE BAR AVEC LES DROITS D'ACCESS------------------------------
    connect(ui->GEmpBtn, &QPushButton::clicked, this, [=]() {
        QString username=ui->empUsernameLineEdit->text();
        if(e.accessRights(username)=="Administrateur" || e.accessRights(username)=="Responsable RH" || e.accessRights(username)=="Chef d’équipe"){
            //ui->gestionsPages->setCurrentIndex(PAGE_GEMPLOYE);
            animatePageTransitionVertical(ui->gestionsPages,PAGE_GEMPLOYE);
            updateSidebarStyles();
        }
    });

    connect(ui->GContBtn, &QPushButton::clicked, this, [=]() {
        QString username=ui->empUsernameLineEdit->text();
        if(e.accessRights(username)!="Chauffeur" && e.accessRights(username)!="Responsable RH"){
            //ui->gestionsPages->setCurrentIndex(PAGE_GCONTAINER);
            animatePageTransitionVertical(ui->gestionsPages,PAGE_GCONTAINER);
            updateSidebarStyles();
        }
    });

    connect(ui->GMachinBtn, &QPushButton::clicked, this, [=]() {
        QString username=ui->empUsernameLineEdit->text();
        if(e.accessRights(username)!="Chauffeur" && e.accessRights(username)!="Responsable RH" && e.accessRights(username)!="Agent de collecte"){
            //ui->gestionsPages->setCurrentIndex(PAGE_GMACHINE);
            animatePageTransitionVertical(ui->gestionsPages,PAGE_GMACHINE);
            updateSidebarStyles();
        }
    });

    connect(ui->GCamBtn, &QPushButton::clicked, this, [=]() {
        QString username=ui->empUsernameLineEdit->text();
        if(e.accessRights(username)!="Responsable RH"){
            //ui->gestionsPages->setCurrentIndex(PAGE_GCAMION);
            animatePageTransitionVertical(ui->gestionsPages,PAGE_GCAMION);
            updateSidebarStyles();
        }

    });

    connect(ui->GPcBtn, &QPushButton::clicked, this, [=]() {
        QString username=ui->empUsernameLineEdit->text();
        //ui->gestionsPages->setCurrentIndex(PAGE_GPOINT_COLLECTE);
        animatePageTransitionVertical(ui->gestionsPages,PAGE_GPOINT_COLLECTE);
        updateSidebarStyles();
    });

    connect(ui->GClientBtn, &QPushButton::clicked, this, [=]() {
        QString username=ui->empUsernameLineEdit->text();
        if(e.accessRights(username)=="Administrateur" || e.accessRights(username)=="Responsable RH" || e.accessRights(username)=="Chef d’équipe"){
            //ui->gestionsPages->setCurrentIndex(PAGE_GCLIENT);
            animatePageTransitionVertical(ui->gestionsPages,PAGE_GCLIENT);
            updateSidebarStyles();
        }
    });

    refreshTable(e);
    showEmployeeStats();
    ui->passwordAddLineEdit->setText(e.get_password());
    connect(ui->employeeAjoutBtn, &QPushButton::clicked, this, [=]() {
        employee e;
        e.set_username(ui->usernameAddLineEdit->text());
        e.set_nom(ui->nomAddLineEdit->text());
        e.set_prenom(ui->prenomAddLineEdit->text());
        e.set_password(ui->passwordAddLineEdit->text());
        e.set_email(ui->emailAddLineEdit->text());
        e.set_poste(ui->employeeComboBox->currentText());
        e.set_telephone(ui->telephoneAddLineEdit->text());
        e.set_question(ui->questionComboBox->currentText());
        e.set_reponse(ui->reponseAddLineEdit->text());
        e.set_rfid(ui->RFIDLabel->text());
        bool test=e.create_employee();
        if(!test){
            qDebug() << "create failed:";
        }else{
             qDebug() << "ajout btn clicked";
        }
        refreshTable(e);
        showEmployeeStats();
    });

    connect(ui->employeeFullTableBtn, &QPushButton::toggled, this, [=](bool checked) {
        if (checked) {
            // ON
            ui->empTableView->hideColumn(3);  // password
            ui->empTableView->hideColumn(4);  // question
            ui->empTableView->hideColumn(6);  // password
            ui->empTableView->hideColumn(4);  // question
            ui->empTableView->hideColumn(8);  // question
            ui->empTableView->hideColumn(9); // reponse
            ui->empTableView->hideColumn(10); // reponse
            ui->empTableView->hideColumn(11);  // id
        }else{
            //off
            ui->empTableView->showColumn(3);  // password
            ui->empTableView->showColumn(4);  // question
            ui->empTableView->showColumn(6);  // password
            ui->empTableView->showColumn(4);  // question
            ui->empTableView->showColumn(8);  // question
            ui->empTableView->showColumn(9);  // question
            ui->empTableView->showColumn(10); // reponse
            ui->empTableView->showColumn(11);  // id
        }
    });
    //search
    connect(ui->employeeSearchBtn, &QPushButton::clicked, this, [=]() {
        QString empSearchWord = ui->employeeSearchLineEdit->text();
        QSqlTableModel *model = (QSqlTableModel*)ui->empTableView->model(); // get the current model
        model->setFilter("USERNAME LIKE '%" + empSearchWord + "%'");
        model->select();
    });
    QSqlTableModel *model = (QSqlTableModel*)ui->empTableView->model(); // get the current model
    connect(model, &QSqlTableModel::dataChanged, this, [=](const QModelIndex &index) {
        QString newValue = index.data().toString();
        int row = index.row();
        int col = index.column();
        std::vector<QString> roles = {
            "Chauffeur",
            "Agent de collecte",
            "Responsable de flotte",
            "Chef d'équipe",
            "Responsable des opérations",
            "Administrateur",
            "Responsable RH",
            "Responsable sécurité",
            "Contrôleur qualité"
        };

        std::vector<QString> questions = {
            "Quel est le nom de votre premier animal ?",
            "Quel est le prénom de votre meilleur ami d'enfance ?",
            "Quel est le nom de votre école primaire ?",
            "Quel est votre plat préféré ?"
        };

        // example: validate phone number (column 5)
        if (col == 5) { // telephone
            QRegularExpression re("^[0-9]+$");
            if (!re.match(newValue).hasMatch()) {
                QMessageBox::warning(nullptr, "Erreur", "Le téléphone doit contenir uniquement des chiffres !");
                model->revert(); // ← cancel the change
                return;
            }
        }

        if (col == 3) { // email
            QRegularExpression re(R"(^[\w\.-]+@[\w\.-]+\.\w{2,}$)");
            if (!re.match(newValue).hasMatch()) {
                QMessageBox::warning(nullptr, "Erreur", "L'adresse email est invalide !");
                model->revert(); // ← cancel the change
                return;
            }
        }

        if (col == 7) { // poste
            if (std::find(roles.begin(),roles.end(),newValue)==roles.end()) {
                QMessageBox::warning(nullptr, "Erreur", "la poste est invalide !");
                model->revert(); // ← cancel the change
                return;
            }
        }

        if (col == 9) { // question
            if (std::find(questions.begin(),questions.end(),newValue)==questions.end()) {
                QMessageBox::warning(nullptr, "Erreur", "le question est invalide !");
                model->revert(); // ← cancel the change
                return;
            }
        }

        if (col == 11) {
            model->revert(); // ← cancel the change
            QMessageBox::warning(nullptr, "Erreur", "vous ne pouvez pas changer l'identifiant !");
            return;
        }
    });
    connect(ui->employeeExportBtn, &QPushButton::clicked, this, [=]() {
        employeeExportToPDF();
    });

    connect(&A, &Arduino::gCliArd, this, [=](){
        A.onDonneesRecues();
    });

    //-----------------------------ANIMATION--------------------------------------
    // setup scene
    // setup scene
    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->empStatBtnGraphicsView->setScene(scene);
    ui->empStatBtnGraphicsView->setStyleSheet("background: transparent; border: none;");
    ui->empStatBtnGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->empStatBtnGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->empStatBtnGraphicsView->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->empStatBtnGraphicsView->setFixedSize(110, 110); // match your icon size
    // wrap the label
    QLabel *rotatingIcon = new QLabel();
    rotatingIcon->setPixmap(QPixmap(":/images/photos/PieBtn.png").scaled(110, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    rotatingIcon->setFixedSize(110, 110);
    rotatingIcon->setStyleSheet("background: transparent;");
    rotatingIcon->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    statBtnProxy = scene->addWidget(rotatingIcon);
    statBtnProxy->setTransformOriginPoint(
        rotatingIcon->width() / 2,
        rotatingIcon->height() / 2
        );
    qDebug() << "proxy:" << statBtnProxy;
    qDebug() << "proxy scene:" << statBtnProxy->scene();

    connect(ui->employeeStackedWidget, &QStackedWidget::currentChanged, this, [=]() {
        ui->empStatBtnGraphicsView->fitInView(statBtnProxy, Qt::KeepAspectRatio);
    });
    connect(ui->gestionsPages, &QStackedWidget::currentChanged, this, [=]() {
        ui->empStatBtnGraphicsView->fitInView(statBtnProxy, Qt::KeepAspectRatio);
    });
    ui->GEmpBtn->installEventFilter(this);
    ui->GClientBtn->installEventFilter(this);
    ui->GPcBtn->installEventFilter(this);
    ui->GCamBtn->installEventFilter(this);
    ui->GMachinBtn->installEventFilter(this);
    ui->GContBtn->installEventFilter(this);
    ui->employeeAjoutBtn->installEventFilter(this);
    ui->employeeExportBtn->installEventFilter(this);
    ui->employeeFullTableBtn->installEventFilter(this);
    ui->employeeStatBtn->installEventFilter(this);
    ui->empStatsBtnLable->installEventFilter(this);
    ui->empStatBtnGraphicsView->installEventFilter(this);
}
void WasteRecycling::updateSidebarStyles() {
    QString defaultStyle = "text-align: left;color: #0e140c;background-color: #baf49c;border-top-right-radius: 25px;border-bottom-right-radius: 25px;ont-weight: bold;padding-left: 10px;font-size: 18pt;";
    QString activeStyle = "color: rgb(255, 255, 255);background-color: #1d4840;border-top-right-radius: 25px;border-bottom-right-radius: 25px;";

    // reset all
    ui->GEmpBtn->setStyleSheet(defaultStyle);
    ui->GClientBtn->setStyleSheet(defaultStyle);
    ui->GPcBtn->setStyleSheet(defaultStyle);
    ui->GCamBtn->setStyleSheet(defaultStyle);
    ui->GMachinBtn->setStyleSheet(defaultStyle);
    ui->GContBtn->setStyleSheet(defaultStyle);

    // activate current
    switch(ui->gestionsPages->currentIndex()) {
    case 0: ui->GEmpBtn->setStyleSheet(activeStyle);    break;
    case 1: ui->GContBtn->setStyleSheet(activeStyle); break;
    case 2: ui->GMachinBtn->setStyleSheet(activeStyle);     break;
    case 3: ui->GCamBtn->setStyleSheet(activeStyle);    break;
    case 4: ui->GPcBtn->setStyleSheet(activeStyle); break;
    case 5: ui->GClientBtn->setStyleSheet(activeStyle);   break;
    }
}
void WasteRecycling::animateMove(QWidget *widget, QPoint endPos, int duration) {
    QPropertyAnimation *anim = new QPropertyAnimation(widget, "pos");
    anim->setDuration(duration);
    anim->setStartValue(widget->pos());
    anim->setEndValue(QPoint(endPos));
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void WasteRecycling::animateFade(QWidget *widget, float startOpacity, float endOpacity, int duration) {
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(duration);
    anim->setStartValue(startOpacity);
    anim->setEndValue(endOpacity);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void WasteRecycling::animateGrow(QWidget *widget, int amount, int duration) {
    // save original geometry if not saved yet
    if (!originalGeometries.contains(widget))
        originalGeometries[widget] = widget->geometry();

    QPropertyAnimation *anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(duration);
    anim->setStartValue(widget->geometry()); // start from CURRENT geometry
    anim->setEndValue(originalGeometries[widget].adjusted(-amount, -amount, amount, amount));
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void WasteRecycling::animateShrink(QWidget *widget, int amount, int duration) {
    if (!originalGeometries.contains(widget))
        originalGeometries[widget] = widget->geometry();

    QPropertyAnimation *anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(duration);
    anim->setStartValue(widget->geometry()); // start from CURRENT geometry
    anim->setEndValue(originalGeometries[widget]); // go back to original
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
void WasteRecycling::animateRotate(QGraphicsProxyWidget *proxy, int startAngle, int endAngle, int duration) {
    QPropertyAnimation *anim = new QPropertyAnimation(proxy, "rotation");
    anim->setDuration(duration);
    anim->setStartValue(startAngle);
    anim->setEndValue(endAngle);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
void WasteRecycling::animatePageTransition(QStackedWidget *stackedWidget, int newIndex) {
    QWidget *currentPage = stackedWidget->currentWidget();
    QWidget *nextPage = stackedWidget->widget(newIndex);

    int width = stackedWidget->width();

    // decide direction
    int direction = (newIndex > stackedWidget->currentIndex()) ? 1 : -1;

    // position next page
    nextPage->setGeometry(direction * width, 0, width, stackedWidget->height());
    stackedWidget->setCurrentIndex(newIndex);

    // current page goes opposite direction
    QPropertyAnimation *animOut = new QPropertyAnimation(currentPage, "pos");
    animOut->setDuration(600);
    animOut->setStartValue(QPoint(0, 0));
    animOut->setEndValue(QPoint(-direction * width, 0));
    animOut->setEasingCurve(QEasingCurve::OutCubic);

    // next page comes in
    QPropertyAnimation *animIn = new QPropertyAnimation(nextPage, "pos");
    animIn->setDuration(600);
    animIn->setStartValue(QPoint(direction * width, 0));
    animIn->setEndValue(QPoint(0, 0));
    animIn->setEasingCurve(QEasingCurve::OutCubic);

    animOut->start(QAbstractAnimation::DeleteWhenStopped);
    animIn->start(QAbstractAnimation::DeleteWhenStopped);
}
void WasteRecycling::animatePageTransitionVertical(QStackedWidget *stackedWidget, int newIndex) {
    QWidget *currentPage = stackedWidget->currentWidget();
    QWidget *nextPage = stackedWidget->widget(newIndex);

    int height = stackedWidget->height();

    // decide direction
    int direction = (newIndex > stackedWidget->currentIndex()) ? 1 : -1;

    // position next page
    nextPage->setGeometry(0, direction * height, stackedWidget->width(), height);
    stackedWidget->setCurrentIndex(newIndex);

    // current page goes up or down
    QPropertyAnimation *animOut = new QPropertyAnimation(currentPage, "pos");
    animOut->setDuration(600);
    animOut->setStartValue(QPoint(0, 0));
    animOut->setEndValue(QPoint(0, -direction * height));
    animOut->setEasingCurve(QEasingCurve::OutCubic);

    // next page comes in
    QPropertyAnimation *animIn = new QPropertyAnimation(nextPage, "pos");
    animIn->setDuration(600);
    animIn->setStartValue(QPoint(0, direction * height));
    animIn->setEndValue(QPoint(0, 0));
    animIn->setEasingCurve(QEasingCurve::OutCubic);

    animOut->start(QAbstractAnimation::DeleteWhenStopped);
    animIn->start(QAbstractAnimation::DeleteWhenStopped);
}
bool WasteRecycling::eventFilter(QObject *obj, QEvent *event) {
    QWidget *widget = qobject_cast<QWidget*>(obj);
    // check WHICH widget
    if (obj == ui->GEmpBtn || obj == ui->GClientBtn || obj == ui->GPcBtn || obj == ui->GCamBtn || obj == ui->GMachinBtn || obj == ui->GContBtn) {

        // check WHAT happened
        if (event->type() == QEvent::Enter) {
            qDebug() << "mouse entered the button";
            animateMove(widget, widget->pos()+QPoint(50,0), 500);

        }
        else if (event->type() == QEvent::Leave) {
            animateMove(widget, QPoint(-50,widget->pos().y()), 500);
            qDebug() << "mouse left the button";
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            qDebug() << "button was clicked";
        }
    }
    if(obj == ui->employeeAjoutBtn || obj == ui->employeeExportBtn || obj == ui->employeeFullTableBtn) {
        if (event->type() == QEvent::Enter) {
            qDebug() << "mouse entered the button";
            animateGrow(widget, 10,300);

        }
        else if (event->type() == QEvent::Leave) {
            animateShrink(widget, 5,600);
            qDebug() << "mouse left the button";
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            qDebug() << "button was clicked";
        }
    }
    if(obj==ui->employeeStatBtn){
        if (event->type() == QEvent::Enter) {
            qDebug() << "mouse entered the button";
            animateRotate(statBtnProxy,0,180,700);
            animateMove(ui->empStatsBtnLable, ui->empStatsBtnLable->pos()+QPoint(40,0), 500);

        }
        else if (event->type() == QEvent::Leave) {
            animateRotate(statBtnProxy,0,180,700);
            animateMove(ui->empStatsBtnLable, QPoint(1030,ui->empStatsBtnLable->pos().y()), 500);
            qDebug() << "mouse left the button";
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            qDebug() << "button was clicked";
        }
    }
    // always return this at the end
    // lets Qt handle everything else normally
    return QMainWindow::eventFilter(obj, event);
}
//-----------------------------ANIMATION--------------------------------------
void WasteRecycling::onRightClick(const QPoint &pos) {
    QModelIndex index = ui->empTableView->indexAt(pos);
    if (!index.isValid()) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Supprimer",
        "Voulez-vous supprimer cet employé ?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QSqlTableModel *model = (QSqlTableModel*)ui->empTableView->model(); // ← get it from the view
        model->removeRow(index.row());
        model->select();
        showEmployeeStats();
    }
}
void WasteRecycling::refreshTable(employee e) {
    QSqlTableModel* model=e.read_employee();
    ui->empTableView->setModel(model);

    ui->empTableView->setSortingEnabled(true);

    model->setSort(11, Qt::AscendingOrder);

    ui->empTableView->setAlternatingRowColors(true);
    // Stretch columns to fill the width
    ui->empTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Fit rows to content
    ui->empTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Hide the row numbers on the left
    ui->empTableView->verticalHeader()->setVisible(false);

    // Single row selection
    ui->empTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->empTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Disable editing by clicking
    //ui->empTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void WasteRecycling::showEmployeeStats() {
    // clear old chart first
    QLayout *oldLayout = ui->employeeStatsFrame->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    employee e;
    QMap<QString, int> stats = e.getPosteStats();

    QPieSeries *series = new QPieSeries();
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        series->append(it.key() + " (" + QString::number(it.value()) + ")", it.value());
    }

    for (QPieSlice *slice : series->slices()) {
        slice->setLabelVisible(true);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Employés par poste");

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(chartView);
    ui->employeeStatsFrame->setLayout(layout);
}
//----------------------------------EXPORT-----------------------------------------------------
void WasteRecycling::employeeExportToPDF() {
    QString filePath = QFileDialog::getSaveFileName(this, "Exporter PDF", "employees.pdf", "PDF Files (*.pdf)");
    if (filePath.isEmpty()) return;

    QSqlTableModel *model = (QSqlTableModel*)ui->empTableView->model();

    // build HTML table
    QString html = "<h2 style='text-align:center;'>Liste des Employés</h2>"
                   "<table border='1' cellspacing='0' cellpadding='6' width='100%'>"
                   "<tr style='background-color:#36656B; color:white;'>";

    // headers
    for (int col = 0; col < model->columnCount(); col++) {
        html += "<th>" + model->headerData(col, Qt::Horizontal).toString() + "</th>";
    }
    html += "</tr>";

    // rows
    for (int row = 0; row < model->rowCount(); row++) {
        QString rowColor = (row % 2 == 0) ? "#ECF5E9" : "#ffffff";
        html += "<tr style='background-color:" + rowColor + ";'>";
        for (int col = 0; col < model->columnCount(); col++) {
            html += "<td>" + model->index(row, col).data().toString() + "</td>";
        }
        html += "</tr>";
    }
    html += "</table>";

    // export
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageOrientation(QPageLayout::Landscape);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Succès", "PDF exporté avec succès !");
}
// ********************************* GESTION EMPLOYEE *****************************************

WasteRecycling::~WasteRecycling()
{
    delete ui;
}
/// //////////////////////////////// DEBUT GESTION CAMION ///////////////////////////////////////

// ******************************** Navigation CAMION ***********************************

void WasteRecycling::on_crudMet1CamBtn_clicked()
{
    QStackedWidget *pagesGcamion = ui->gestionsPages
                                       ->findChild<QWidget*>("GCamionPage")
                                       ->findChild<QStackedWidget*>("pagesGcamion");

    if (pagesGcamion) {
        pagesGcamion->setCurrentIndex(PAGE_CAMION_F1);
    } else {
        qDebug() << "Navigation échouée";
    }
}

void WasteRecycling::on_crudMet2CamBtn_clicked()
{
    QStackedWidget *pagesGcamion = ui->gestionsPages
                                       ->findChild<QWidget*>("GCamionPage")
                                       ->findChild<QStackedWidget*>("pagesGcamion");

    if (pagesGcamion) {
        pagesGcamion->setCurrentIndex(PAGE_CAMION_F2);
    } else {
        qDebug() << "Navigation échouée";
    }
}

void WasteRecycling::on_f1AccCamBtn_clicked()
{
    QStackedWidget *pagesGcamion = ui->gestionsPages
                                       ->findChild<QWidget*>("GCamionPage")
                                       ->findChild<QStackedWidget*>("pagesGcamion");

    if (pagesGcamion) {
        pagesGcamion->setCurrentIndex(PAGE_CAMION_CRUD);
    } else {
        qDebug() << "Navigation échouée";
    }
}

void WasteRecycling::on_f1Met2CamBtn_clicked()
{
    QStackedWidget *pagesGcamion = ui->gestionsPages
                                       ->findChild<QWidget*>("GCamionPage")
                                       ->findChild<QStackedWidget*>("pagesGcamion");

    if (pagesGcamion) {
        pagesGcamion->setCurrentIndex(PAGE_CAMION_F2);
    } else {
        qDebug() << "Navigation échouée";
    }
}

void WasteRecycling::on_f2AccCamBtn_clicked()
{
    QStackedWidget *pagesGcamion = ui->gestionsPages
                                       ->findChild<QWidget*>("GCamionPage")
                                       ->findChild<QStackedWidget*>("pagesGcamion");

    if (pagesGcamion) {
        pagesGcamion->setCurrentIndex(PAGE_CAMION_CRUD);
    } else {
        qDebug() << "Navigation échouée";
    }
}

void WasteRecycling::on_f2Met1CamBtn_clicked()
{
    QStackedWidget *pagesGcamion = ui->gestionsPages
                                       ->findChild<QWidget*>("GCamionPage")
                                       ->findChild<QStackedWidget*>("pagesGcamion");

    if (pagesGcamion) {
        pagesGcamion->setCurrentIndex(PAGE_CAMION_F1);
    } else {
        qDebug() << "Navigation échouée";
    }
}

// ******************************* Fin Navigation Camion *******************************************



// Fonctionalité supplementaire pour vider le formulaire de modification et d'ajout
void WasteRecycling::resetAddEditCamForm()
{
    ui->matCamInp->clear(); // Vider le LineEdit du Matricule
    ui->capCamInp->clear(); // Vider le LineEdit de la Capacite
    ui->conCamInp->clear(); // Vider le LineEdit de la Consomation
    ui->etaCamInp->setCurrentText("Disponible"); // initialiser le QComboBox à disponible
    ui->addCamBtn->setText("Ajouter"); // Modification du texte du bouton
    CamTemp.setCamEditMode(false); // desactiver le mode modificatino
}

// **************************** Fonctionalité CRUD **************************************

/// Ajout / Modification
void WasteRecycling::on_addCamBtn_clicked()
{
    bool ok1,ok2;
    QString mat = ui->matCamInp->text();
    float capacite = ui->capCamInp->text().toFloat(&ok1); // &ok1 verifie que le champs est remplie et valide
    float conso = ui->conCamInp->text().toFloat(&ok2); // &ok2 verifie que le champs est remplie et valide
    QString etat = ui->etaCamInp->currentText();
    Camion C(mat,capacite,0,conso,etat);
    // verifier si tous les champs sont non nulle et valide
    if(!ok1 || !ok2 || mat == "")
    {
        QMessageBox::critical(
            nullptr,
            QMessageBox::tr("Erreur Saisie"),
            QMessageBox::tr("Veiller remplir tous les champs avec des données valide!")
            );
    }else if(C.camControleSaisie())
    {
        if(CamTemp.getCamEditMode()) // Mode Modification du camion
        {

            QMessageBox::StandardButton reponse = QMessageBox::question(
                this,
                QMessageBox::tr("Confirmation"),
                QMessageBox::tr("Voulez-vous vraiment modifier le camion ?"),
                QMessageBox::Yes | QMessageBox::No
                );

            if(reponse == QMessageBox::Yes)
            {
                // Modification du Camion Temporaire pour la mise à jour
                CamTemp.setMatricule(mat);
                CamTemp.setCapacite_max(capacite);
                CamTemp.setConsomation(conso);
                CamTemp.setStatus(etat);
                bool test = CamTemp.editCamion();
                if(test)
                {
                    ui->tableCam->setModel(CamTemp.showCamion());// refresh
                    WasteRecycling::resetAddEditCamForm();

                }
                else
                {
                    QMessageBox::critical(
                        nullptr,QObject::tr("Probleme Survenu"),
                        QObject::tr("Modification non effectué.\nClick cancel to exit")
                        );
                }
            }
            else
            {
                WasteRecycling::resetAddEditCamForm();
            }
        }
        else{ // Mode ajout du camion

            if(C.fetchCamionByMatricule(mat))
            {
                QMessageBox::critical(
                    nullptr,
                    QMessageBox::tr("Erreur saisie"),
                    QMessageBox::tr("le Matricule saisie existe deja!!")
                    );
            }
            else
            {
                C.setMatricule(mat);
                C.setCapacite_max(capacite);
                C.setRemplissage(0);
                C.setConsomation(conso);
                C.setStatus(etat);
                QMessageBox::StandardButton reponse = QMessageBox::question(
                    this,
                    QMessageBox::tr("Confirmation"),
                    QMessageBox::tr("Voulez-vous vraiment ajouter ce camion ?"),
                    QMessageBox::Yes | QMessageBox::No
                    );
                if(reponse == QMessageBox::Yes)
                {
                    bool test = C.addCamion();
                    if(test)
                    {
                        ui->tableCam->setModel(CamTemp.showCamion());// refresh
                        WasteRecycling::resetAddEditCamForm();
                    }
                    else
                    {
                        QMessageBox::critical(
                            nullptr,QObject::tr("Probleme Survenu"),
                            QObject::tr("Ajout non effectué.\nClick cancel to exit")
                            );
                    }
                }
            }
        }
    }
}

/// Suppression
void WasteRecycling::on_delCamBtn_clicked()
{
    QString mat = ui->delCamInp->text();
    // confirmation de la suppression
    QMessageBox::StandardButton response = QMessageBox::question(
        this,
        QMessageBox::tr("Confirmation"),
        QMessageBox::tr("Voulez-vous vraiment supprimer ce camion ?"),
        QMessageBox::Yes | QMessageBox::No
        );
    if(response == QMessageBox::Yes)
    {
        bool test = CamTemp.deleteCamion(mat);
        if(test)
        {
            ui->tableCam->setModel(CamTemp.showCamion()); // refresh
            ui->delCamInp->clear();
        }
        else
        {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Probleme Survenu"),
                QObject::tr("Suppression non effectué.\nClick cancel to exit")
                );
        }
    }
}

/// Modification
void WasteRecycling::on_tableCam_doubleClicked(const QModelIndex &index)
{

    if(!index.isValid()) return;
    if(!CamTemp.getCamEditMode())
    {

        int row = index.row();
        QAbstractItemModel *model = ui->tableCam->model();

        // Extraction des information à partir du tableau
        QString matricule = model->data(model->index(row,0)).toString();
        QString capacite = QString::number(model->data(model->index(row,1)).toFloat(),'f',1);
        QString conso = QString::number(model->data(model->index(row,3)).toFloat(),'f',1);
        QString etat = model->data(model->index(row,4)).toString();

        // va afficher ces information dans le formulaire
        ui->matCamInp->setText(matricule);
        ui->capCamInp->setText(capacite);
        ui->conCamInp->setText(conso);
        ui->etaCamInp->setCurrentText(etat);
        ui->addCamBtn->setText("Modifier");
        CamTemp.fetchCamionByMatricule(matricule);
        CamTemp.setCamEditMode(true);
    }else
    {
        WasteRecycling::resetAddEditCamForm();
    }
}
// ******************************* Fin Fonctionalité crud *******************************


/// Recherche
void WasteRecycling::on_rechCamBtn_clicked()
{
    QString mat = ui->rechCamInp->text().trimmed();
    if(mat == "")
    {
        ui->tableCam->setModel(CamTemp.showCamion());
    }
    else
    {
        ui->tableCam->setModel(CamTemp.camRechMat(mat));
    }
}

// Metiers simples camion(Recherche, tri, PDF, Statistique)

void WasteRecycling::camMatSearch(const QString &text)
{
    if(text.trimmed().isEmpty())
    {
        ui->tableCam->setModel(CamTemp.showCamion());
    }
    else
    {
        ui->tableCam->setModel(CamTemp.camRechMat(text));
    }
}

void WasteRecycling::on_triCamBtn_clicked()
{
    CamTemp.setTrieAsc(!CamTemp.getTrieAsc());
    QString mat = ui->rechCamInp->text().trimmed();
    ui->tableCam->setModel(CamTemp.camTrierStatus(mat));
}

void WasteRecycling::on_pdfCamBtn_clicked()
{
    CamTemp.camExportPDF(CamTemp.showCamion());
}

void WasteRecycling::on_statCamBtn_clicked()
{
    CamTemp.afficherStatistiques();
}

// Map
void WasteRecycling::mettreAJourLaCarte(const QList<QGeoCoordinate>& listePoints) {
    QObject *root = ui->camMapShow->rootObject();
    if (root) {
        // 1. On vide la carte
        QMetaObject::invokeMethod(root, "clearMap");

        // 2. On ajoute les nouveaux points de collecte
        for (const auto& coord : listePoints) {
            QMetaObject::invokeMethod(root, "addPoint",
                                      Q_ARG(QVariant, coord.latitude()),
                                      Q_ARG(QVariant, coord.longitude()));
        }
    }
}
void WasteRecycling::on_camZoomBtn_clicked()
{
    CamTemp.camMapZoom(ui->camMapShow,true);
}

void WasteRecycling::on_camDezoomBtn_clicked()
{
    CamTemp.camMapZoom(ui->camMapShow,false);
}

void WasteRecycling::on_camCentreBtn_clicked()
{
    CamTemp.camMapCentrer(ui->camMapShow);
}

void WasteRecycling::on_camMapRaf_clicked()
{
    CamTemp.camMapShowPC(ui->camMapShow);
}

void WasteRecycling::on_camItinBtn_clicked()
{
    if(ui->camItinBtn->text() == "Lancer l'itineraire"){
        QString matricule = ui->camMatItiInp->text().trimmed();
        if(matricule.isEmpty()) {
            QMessageBox::warning(this,"Erreur", "Veillez entrer un matricule.");
            return;
        }

        if(!CamTemp.fetchCamionByMatricule(matricule)
            || CamTemp.getStatus() == "Maintenance"){
            QMessageBox::warning(this, "Erreur", "Matricule du Camion introuvable: ou en maintenance"+ matricule);
            return;
        }

        qDebug() << "Camion trouvé, id_camion = " << CamTemp.getId_camion();
        m_routeMgr->computeRoute(CamTemp.getId_camion());
        ui->camItinBtn->setText("Arreter l'itineraire");
    }else{
        QMetaObject::invokeMethod(ui->camMapShow->rootObject(), "clearRoute");
        ui->camDistOut->setText("-- km");
        ui->camDurOut->setText("-- min");
        ui->camMatItiInp->clear();
        ui->camItinBtn->setText("Lancer l'itineraire");
        CamTemp.camMapShowPC(ui->camMapShow);
    }
}

void WasteRecycling::on_camAffTable_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid()) return;

    int row = index.row();
    QAbstractItemModel *model = ui->camAffTable->model();

    // Extraction des information à partir du tableau
    QString matricule = model->data(model->index(row,0)).toString();
    QString nom = model->data(model->index(row,1)).toString();
    QString prenom = model->data(model->index(row,2)).toString();
    // va afficher ces information dans le formulaire
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Voulez-vous vraiment Supprimer L'affectation "+ matricule + ", "+ nom +", "+ prenom +"?",
                                  QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        if(!AffTemp.fetchAffectPK(nom,prenom,matricule))
        {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Probleme Survenu"),
                QObject::tr("Impossible de trouver les clees.")
                );
            return;
        }
        if(!AffTemp.affectationDelete())
        {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Probleme Survenu"),
                QObject::tr("Suppression non effectué.\nClick cancel to exit"));
            return;
        }

        if(!AffTemp.affFreeCamion())
        {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Probleme Survenu"),
                QObject::tr("Probleme de modification de base"));
            return;
        }
        ui->camAffTable->setModel(AffTemp.affectationShow());
        ui->tableCam->setModel(CamTemp.showCamion());
    }
}

void WasteRecycling::on_camAffPropBtn_clicked()
{
    if(!AffTemp.affProposerAffectation())
    {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Probleme Survenu"),
            QObject::tr("Proposition impossible"));
        return;
    }

    ui->camAffMatInp->setText(AffTemp.Cam.getMatricule());
    ui->camAffNomInp->setText(AffTemp.emp.get_nom());
    ui->camAffPrenomInp->setText(AffTemp.emp.get_prenom());

}

void WasteRecycling::on_camAffAddBtn_clicked()
{

    QString mat = ui->camAffMatInp->text();
    QString nom = ui->camAffNomInp->text();
    QString prenom = ui->camAffPrenomInp->text();
    if(!AffTemp.fetchAffectPK(nom,prenom,mat))
    {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Probleme Survenu"),
            QObject::tr("Camion ou employé indisponible ou l'employé n'est pas un chauffeur"));
        return;
    }

    if(AffTemp.affCheckFreeCamion() && AffTemp.affCheckFreeEmployee())
    {
        if(!AffTemp.affecterCamionEmployee()){
            QMessageBox::critical(
                nullptr,
                QObject::tr("Probleme Survenu"),
                QObject::tr("Ajout Non effectué "));
            return;
        }
        AffTemp.affUseCamion();
        ui->camAffTable->setModel(AffTemp.affectationShow());
        ui->tableCam->setModel(CamTemp.showCamion());
    }else{
        QMessageBox::critical(
            nullptr,
            QObject::tr("Probleme Survenu"),
            QObject::tr("Les Camions ou employee suggéré sont \nInexistant ou indisponible"));
        return;
    }

}

void WasteRecycling::on_camShowRempBtn_clicked()
{
    QString mat = ui->camShowRempInp->text();
    CamTemp.fetchCamionByMatricule(mat);
    A.send_remplissage(CamTemp.getRemplissage());
}


/// ///////////////////////// FIN GESTION CAMION //////////////////////////////////


// ********************************* GESTION CONTENEURS *****************************************



// ==================== HELPER METHODS ====================

void WasteRecycling::refreshTable()
{
    // Get all containers using the DAO method
    QVector<Conteneur> conteneurs = Conteneur::getAllConteneurs();

    // Create a custom model to show predictions
    QStandardItemModel *model = new QStandardItemModel(conteneurs.size(), 8, this);

    // Set headers
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Type");
    model->setHeaderData(2, Qt::Horizontal, "Capacite Max");
    model->setHeaderData(3, Qt::Horizontal, "Quantite Actuelle");
    model->setHeaderData(4, Qt::Horizontal, "Remplissage");
    model->setHeaderData(5, Qt::Horizontal, "Niveau Alerte");
    model->setHeaderData(6, Qt::Horizontal, "Date MAJ");
    model->setHeaderData(7, Qt::Horizontal, "Prediction");

    // Populate data
    for (int i = 0; i < conteneurs.size(); ++i) {
        const Conteneur& c = conteneurs[i];
        int percent = c.getPourcentageRemplissage();
        QString alert = c.getNiveauAlerte();
        QString prediction = c.getTimeUntilFull();
        int daysUntilFull = c.getDaysUntilFull();

        // Set items
        model->setItem(i, 0, new QStandardItem(QString::number(c.getId())));
        model->setItem(i, 1, new QStandardItem(c.getType()));
        model->setItem(i, 2, new QStandardItem(QString::number(c.getCapaciteMax()) + " L"));
        model->setItem(i, 3, new QStandardItem(QString::number(c.getQuantiteActuelle()) + " L"));

        // For percentage
        QStandardItem *percentItem = new QStandardItem(QString::number(percent) + "%");
        model->setItem(i, 4, percentItem);

        QStandardItem *alertItem = new QStandardItem(alert);
        // Color code alert levels
        if (alert == "Critique") {
            alertItem->setForeground(QBrush(QColor(231, 76, 60)));
        } else if (alert == "Avertissement") {
            alertItem->setForeground(QBrush(QColor(243, 156, 18)));
        } else {
            alertItem->setForeground(QBrush(QColor(39, 174, 96)));
        }
        model->setItem(i, 5, alertItem);

        model->setItem(i, 6, new QStandardItem(c.getDateMaj().toString("dd/MM/yyyy")));

        QStandardItem *predictionItem = new QStandardItem(prediction);
        // Color code prediction based on urgency
        if (daysUntilFull >= 0 && daysUntilFull <= 2) {
            predictionItem->setForeground(QBrush(QColor(231, 76, 60))); // Red - urgent
            predictionItem->setText(prediction + " (Urgent)");
        } else if (daysUntilFull >= 0 && daysUntilFull <= 5) {
            predictionItem->setForeground(QBrush(QColor(243, 156, 18))); // Orange - warning
        } else if (daysUntilFull > 0) {
            predictionItem->setForeground(QBrush(QColor(39, 174, 96))); // Green - safe
        }
        model->setItem(i, 7, predictionItem);
    }

    ui->GCont_containerTable->setModel(model);

    // Enable sorting on the model
    model->setSortRole(Qt::DisplayRole);

    // Adjust column widths
    ui->GCont_containerTable->horizontalHeader()->setStretchLastSection(true);
    ui->GCont_containerTable->resizeColumnsToContents();
}

void WasteRecycling::clearForm()
{
    ui->GCont_idInput->clear();
    ui->GCont_typeCombo->setCurrentIndex(0);
    ui->GCont_capacitySpin->setValue(500);
    ui->GCont_currentSpin->setValue(250);
    ui->GCont_thresholdSpin->setValue(80);
    ui->GCont_dateInput->setDate(QDate::currentDate());

    ui->GCont_previewText->setPlainText("Formulaire efface. Entrez de nouvelles informations.");
    ui->GCont_idInput->setFocus();
}

bool WasteRecycling::validateForm()
{
    // Controle de saisie obligatoire
    if (ui->GCont_idInput->text().isEmpty()) {
        QMessageBox::warning(this, "Information manquante", "Veuillez saisir un ID de conteneur.");
        ui->GCont_idInput->setFocus();
        return false;
    }

    bool ok;
    ui->GCont_idInput->text().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "ID invalide", "L'ID doit etre un nombre.");
        ui->GCont_idInput->setFocus();
        return false;
    }

    if (ui->GCont_currentSpin->value() > ui->GCont_capacitySpin->value()) {
        QMessageBox::warning(this, "Donnees invalides",
                             "Le niveau actuel ne peut pas depasser la capacite maximale.");
        ui->GCont_currentSpin->setValue(ui->GCont_capacitySpin->value());
        return false;
    }

    return true;
}

Conteneur WasteRecycling::getConteneurFromForm()
{
    int id = ui->GCont_idInput->text().toInt();
    QString type = ui->GCont_typeCombo->currentText();
    int capacite = ui->GCont_capacitySpin->value();
    int quantite = ui->GCont_currentSpin->value();
    int seuil = ui->GCont_thresholdSpin->value();
    QDate date = ui->GCont_dateInput->date();

    return Conteneur(id, type, capacite, quantite, seuil, date);
}

void WasteRecycling::fillFormFromConteneur(const Conteneur& conteneur)
{
    ui->GCont_idInput->setText(QString::number(conteneur.getId()));
    ui->GCont_typeCombo->setCurrentText(conteneur.getType());
    ui->GCont_capacitySpin->setValue(conteneur.getCapaciteMax());
    ui->GCont_currentSpin->setValue(conteneur.getQuantiteActuelle());
    ui->GCont_thresholdSpin->setValue(conteneur.getSeuilAlerte());
    ui->GCont_dateInput->setDate(conteneur.getDateMaj());

    // Show preview
    on_GCont_previewButton_clicked();
}

int WasteRecycling::getSelectedId()
{
    QModelIndex index = ui->GCont_containerTable->currentIndex();
    if (!index.isValid()) {
        return -1;
    }

    int row = index.row();
    QVariant data = ui->GCont_containerTable->model()->data(
        ui->GCont_containerTable->model()->index(row, 0));
    return data.toInt();
}

// ==================== CRUD OPERATIONS ====================

// CREATE - AJOUT
void WasteRecycling::on_GCont_addButton_clicked()
{
    if (!validateForm()) return;

    // Recuperer les donnees du formulaire
    Conteneur nouveau = getConteneurFromForm();

    // Verifier si l'ID existe deja
    if (Conteneur::idExiste(nouveau.getId())) {
        QMessageBox::critical(this, "Erreur",
                              "Echec de l'ajout du conteneur.\n"
                              "L'ID " + QString::number(nouveau.getId()) + " existe deja dans la base de donnees.\n"
                                                                       "Veuillez utiliser un ID unique.");
        return;
    }

    // Appel de la methode ajouter() du modele
    bool test = nouveau.ajouter();

    if (test) {
        // Refresh - Actualiser l'affichage
        refreshTable();
        clearForm();

        QMessageBox::information(this, "Succès",
                                 QString("Conteneur ajoute avec succes!\n\n"
                                         "ID: %1\nType: %2\nNiveau: %3%")
                                     .arg(nouveau.getId())
                                     .arg(nouveau.getType())
                                     .arg(nouveau.getPourcentageRemplissage()));

    } else {
        // Check if the database is accessible
        Connection& conn = Connection::createInstance();
        QSqlDatabase db = conn.getDatabase();

        if (!db.isOpen()) {
            QMessageBox::critical(this, "Erreur de connexion",
                                  "La connexion a la base de donnees est perdue.\n"
                                  "Veuillez redemarrer l'application.");
        } else {
            QMessageBox::critical(this, "Erreur",
                                  "Echec de l'ajout du conteneur.\n"
                                  "Verifiez que la table CONTENEURS existe dans la base de donnees.");
        }
    }
}

// READ - RAFRAICHIR
void WasteRecycling::on_GCont_refreshButton_clicked()
{
    refreshTable();
}

// UPDATE - MODIFIER
void WasteRecycling::on_GCont_updateButton_clicked()
{
    int id = getSelectedId();
    if (id == -1) {
        QMessageBox::warning(this, "Selection", "Veuillez selectionner un conteneur a modifier.");
        return;
    }

    if (!validateForm()) return;

    Conteneur modifie = getConteneurFromForm();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Voulez-vous vraiment modifier le conteneur ID: " + QString::number(id) + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        bool test = modifie.modifier(id);

        if (test) {
            refreshTable();
            clearForm();
            QMessageBox::information(this, "Succès", "Conteneur modifie avec succes!");
        } else {
            QMessageBox::critical(this, "Erreur", "Echec de la modification.");
        }
    }
}

// DELETE - SUPPRIMER
void WasteRecycling::on_GCont_deleteButton_clicked()
{
    int id = getSelectedId();
    if (id == -1) {
        QMessageBox::warning(this, "Selection", "Veuillez selectionner un conteneur a supprimer.");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Voulez-vous vraiment supprimer le conteneur ID: " + QString::number(id) + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Creer un objet temporaire pour appeler supprimer
        Conteneur temp;
        bool test = temp.supprimer(id);

        if (test) {
            // Refresh - Actualiser l'affichage
            refreshTable();
            clearForm();
            QMessageBox::information(this, "Succès", "Conteneur supprime avec succes!");
        } else {
            QMessageBox::critical(this, "Erreur", "Echec de la suppression.");
        }
    }
}

// ==================== UI SLOTS ====================

void WasteRecycling::on_GCont_clearButton_clicked()
{
    clearForm();
}

void WasteRecycling::on_GCont_previewButton_clicked()
{
    if (ui->GCont_idInput->text().isEmpty()) {
        ui->GCont_previewText->setPlainText("Veuillez saisir un ID de conteneur.");
        return;
    }

    Conteneur conteneur = getConteneurFromForm();

    QString preview = "=== APERCU DU CONTENEUR ===\n\n";
    preview += "ID: " + QString::number(conteneur.getId()) + "\n";
    preview += "Type: " + conteneur.getType() + "\n";
    preview += "Capacite: " + QString::number(conteneur.getCapaciteMax()) + " L\n";
    preview += "Niveau actuel: " + QString::number(conteneur.getQuantiteActuelle()) + " L\n";
    preview += "Pourcentage: " + QString::number(conteneur.getPourcentageRemplissage()) + "%\n";
    preview += "Niveau d'alerte: " + conteneur.getNiveauAlerte() + "\n";
    preview += "Seuil d'alerte: " + QString::number(conteneur.getSeuilAlerte()) + "%\n";
    preview += "Date: " + conteneur.getDateMaj().toString("dd/MM/yyyy") + "\n";

    ui->GCont_previewText->setPlainText(preview);
}

void WasteRecycling::on_GCont_sortButton_clicked()
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->GCont_containerTable->model());

    if (model) {
        // Sort by column 1 (Type) in ascending order
        model->sort(1, Qt::AscendingOrder);
    } else {
        // Fallback for other model types
        ui->GCont_containerTable->sortByColumn(1, Qt::AscendingOrder);
    }
}

void WasteRecycling::on_GCont_searchButton_clicked()
{
    QString searchText = ui->GCont_searchEdit->text();
    QString typeFilter = ui->GCont_typeFilterCombo->currentText();

    QVector<Conteneur> results;

    if (!searchText.isEmpty()) {
        results = Conteneur::rechercherConteneurs(searchText);
    } else if (typeFilter != "Tous les types") {
        results = Conteneur::filtrerParType(typeFilter);
    } else {
        results = Conteneur::getAllConteneurs();
    }

    // Update the table with results
    QStandardItemModel *model = new QStandardItemModel(results.size(), 8, this);

    // Set headers
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Type");
    model->setHeaderData(2, Qt::Horizontal, "Capacite Max");
    model->setHeaderData(3, Qt::Horizontal, "Quantite Actuelle");
    model->setHeaderData(4, Qt::Horizontal, "Remplissage");
    model->setHeaderData(5, Qt::Horizontal, "Niveau Alerte");
    model->setHeaderData(6, Qt::Horizontal, "Date MAJ");
    model->setHeaderData(7, Qt::Horizontal, "Prediction");

    // Populate data
    for (int i = 0; i < results.size(); ++i) {
        const Conteneur& c = results[i];
        int percent = c.getPourcentageRemplissage();
        QString alert = c.getNiveauAlerte();
        QString prediction = c.getTimeUntilFull();
        int daysUntilFull = c.getDaysUntilFull();

        model->setItem(i, 0, new QStandardItem(QString::number(c.getId())));
        model->setItem(i, 1, new QStandardItem(c.getType()));
        model->setItem(i, 2, new QStandardItem(QString::number(c.getCapaciteMax()) + " L"));
        model->setItem(i, 3, new QStandardItem(QString::number(c.getQuantiteActuelle()) + " L"));
        model->setItem(i, 4, new QStandardItem(QString::number(percent) + "%"));

        QStandardItem *alertItem = new QStandardItem(alert);
        if (alert == "Critique") {
            alertItem->setForeground(QBrush(QColor(231, 76, 60)));
        } else if (alert == "Avertissement") {
            alertItem->setForeground(QBrush(QColor(243, 156, 18)));
        } else {
            alertItem->setForeground(QBrush(QColor(39, 174, 96)));
        }
        model->setItem(i, 5, alertItem);

        model->setItem(i, 6, new QStandardItem(c.getDateMaj().toString("dd/MM/yyyy")));

        QStandardItem *predictionItem = new QStandardItem(prediction);
        if (daysUntilFull >= 0 && daysUntilFull <= 2) {
            predictionItem->setForeground(QBrush(QColor(231, 76, 60)));
            predictionItem->setText(prediction + " (Urgent)");
        } else if (daysUntilFull >= 0 && daysUntilFull <= 5) {
            predictionItem->setForeground(QBrush(QColor(243, 156, 18)));
        } else if (daysUntilFull > 0) {
            predictionItem->setForeground(QBrush(QColor(39, 174, 96)));
        }
        model->setItem(i, 7, predictionItem);
    }

    ui->GCont_containerTable->setModel(model);
    model->setSortRole(Qt::DisplayRole);
    ui->GCont_containerTable->resizeColumnsToContents();

    int count = results.size();
}

// NEW: Single click handler for Arduino
void WasteRecycling::on_GCont_containerTable_clicked(const QModelIndex &index)
{
    // Get the selected row and ID
    int row = index.row();
    QVariant idData = ui->GCont_containerTable->model()->data(
        ui->GCont_containerTable->model()->index(row, 0));
    int selectedId = idData.toInt();

    if (selectedId <= 0) {
        qDebug() << "Invalid ID selected:" << selectedId;
        return;
    }

    // Query the database using Conteneur class
    Conteneur selectedConteneur = Conteneur::getConteneurById(selectedId);

    if (selectedConteneur.getId() != 0) {
        int fillPercent = selectedConteneur.getPourcentageRemplissage();
        QString type = selectedConteneur.getType();

        // Send to Arduino
        A.sendToArduino(selectedId, type, fillPercent);

    } else {
        qDebug() << "Container not found with ID:" << selectedId;
    }
}

void WasteRecycling::on_GCont_containerTable_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
    QVariant data = ui->GCont_containerTable->model()->data(
        ui->GCont_containerTable->model()->index(row, 0));
    int id = data.toInt();

    Conteneur conteneur = Conteneur::getConteneurById(id);

    if (conteneur.getId() != 0) {
        fillFormFromConteneur(conteneur);
    }
}

// ==================== PDF GENERATION METHODS ====================

QString WasteRecycling::getCurrentDateTime()
{
    return QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
}

void WasteRecycling::on_GCont_exportPDFButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Export PDF",
                                  "Voulez-vous exporter tous les conteneurs vers un fichier PDF?\n\n"
                                  "Le rapport inclura:\n"
                                  "- Liste complete des conteneurs\n"
                                  "- Statistiques detailles\n"
                                  "- Niveaux d'alerte\n"
                                  "- Date d'exportation",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        generatePDFReport();
    }
}

void WasteRecycling::generatePDFReport()
{
    // Create progress dialog
    QProgressDialog progress("Generation du rapport PDF...", "Annuler", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(10);

    // Get all containers using the DAO method
    QVector<Conteneur> conteneurs = Conteneur::getAllConteneurs();

    progress.setValue(30);

    if (conteneurs.isEmpty()) {
        QMessageBox::warning(this, "Aucune donnee", "Il n'y a aucun conteneur dans la base de donnees.");
        return;
    }

    // Ask user where to save the PDF
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Enregistrer le rapport PDF",
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Rapport_Conteneurs_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"),
                                                    "PDF Files (*.pdf)");

    if (fileName.isEmpty()) {
        return;
    }

    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf";
    }

    progress.setValue(50);

    // Calculate statistics
    int total = conteneurs.size();
    int normal = 0, warning = 0, critical = 0;
    double avgFillPercent = 0;

    for (const Conteneur& c : conteneurs) {
        QString alert = c.getNiveauAlerte();
        if (alert == "Critique") critical++;
        else if (alert == "Avertissement") warning++;
        else normal++;
        avgFillPercent += c.getPourcentageRemplissage();
    }
    avgFillPercent = (total > 0) ? avgFillPercent / total : 0;

    // Create HTML content for the PDF
    QString html;

    // HTML header
    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 40px; }";
    html += "h1 { color: #245F52; text-align: center; font-size: 24px; }";
    html += "h2 { color: #245F52; font-size: 18px; margin-top: 30px; }";
    html += ".date { text-align: center; color: gray; font-size: 12px; margin-bottom: 30px; }";
    html += ".stats-box { background-color: #F0F8F0; border: 1px solid #B7CDB5; border-radius: 10px; padding: 15px; margin: 20px 0; }";
    html += ".stats-box p { margin: 8px 0; font-size: 14px; }";
    html += "table { width: 100%; border-collapse: collapse; margin-top: 20px; }";
    html += "th { background-color: #245F52; color: white; padding: 12px; text-align: center; border: 1px solid #ddd; }";
    html += "td { padding: 10px; text-align: center; border: 1px solid #ddd; }";
    html += "tr:nth-child(even) { background-color: #f9f9f9; }";
    html += ".progress-bar-container { background-color: #e0e0e0; border-radius: 5px; height: 20px; width: 100%; position: relative; }";
    html += ".progress-bar-fill { height: 20px; border-radius: 5px; text-align: center; line-height: 20px; color: white; font-size: 12px; }";
    html += ".fill-normal { background-color: #27AE60; }";
    html += ".fill-warning { background-color: #F39C12; }";
    html += ".fill-critical { background-color: #E74C3C; }";
    html += ".alert-normal { color: #27AE60; font-weight: bold; }";
    html += ".alert-warning { color: #F39C12; font-weight: bold; }";
    html += ".alert-critical { color: #E74C3C; font-weight: bold; }";
    html += ".footer { text-align: center; color: gray; font-size: 10px; margin-top: 40px; }";
    html += "</style>";
    html += "</head>";
    html += "<body>";

    // Title
    html += "<h1>RAPPORT DES CONTENEURS</h1>";

    // Table headers
    html += "<h2>LISTE DES CONTENEURS</h2>";
    html += "<table>";
    html += "<thead>";
    html += "<tr>";
    html += "<th>ID</th>";
    html += "<th>Type</th>";
    html += "<th>Capacite Max</th>";
    html += "<th>Quantite Actuelle</th>";
    html += "<th>Remplissage</th>";
    html += "<th>Niveau Alerte</th>";
    html += "<th>Prediction</th>";
    html += "<th>Date MAJ</th>";
    html += "</tr>";
    html += "</thead>";
    html += "<tbody>";

    // Table data
    for (const Conteneur& c : conteneurs) {
        int percent = c.getPourcentageRemplissage();
        QString alert = c.getNiveauAlerte();
        QString prediction = c.getTimeUntilFull();

        QString progressClass;
        QString alertClass;
        if (percent >= 90) {
            progressClass = "fill-critical";
            alertClass = "alert-critical";
        } else if (percent >= c.getSeuilAlerte()) {
            progressClass = "fill-warning";
            alertClass = "alert-warning";
        } else {
            progressClass = "fill-normal";
            alertClass = "alert-normal";
        }

        html += "<tr>";
        html += "<td style='text-align: center;'>" + QString::number(c.getId()) + "</td>";
        html += "<td style='text-align: left;'>" + c.getType() + "</td>";
        html += "<td style='text-align: right;'>" + QString::number(c.getCapaciteMax()) + " L</td>";
        html += "<td style='text-align: right;'>" + QString::number(c.getQuantiteActuelle()) + " L</td>";

        html += "<td style='width: 120px;'>";
        html += "<div class='progress-bar-container'>";
        html += "<div class='progress-bar-fill " + progressClass + "' style='width: " + QString::number(percent) + "%;'>" + QString::number(percent) + "%</div>";
        html += "</div>";
        html += "</td>";

        html += "<td style='text-align: center;'><span class='" + alertClass + "'>" + alert + "</span></td>";
        html += "<td style='text-align: center;'>" + prediction + "</td>";
        html += "<td style='text-align: center;'>" + c.getDateMaj().toString("dd/MM/yyyy") + "</td>";
        html += "</tr>";
    }

    html += "</tbody>";
    html += "<table>";

    html += "<div class='footer'>Rapport genere automatiquement</div>";
    html += "</body>";
    html += "</html>";

    progress.setValue(70);

    // Create PDF using QTextDocument
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(20, 20, 20, 20));

    QTextDocument doc;
    doc.setHtml(html);
    doc.setDefaultFont(QFont("Arial", 10));
    doc.print(&printer);

    progress.setValue(100);

    QMessageBox::information(this, "Succès",
                             QString("Rapport PDF genere avec succes!\n\n"
                                     "Fichier: %1\n"
                                     "Nombre de conteneurs: %2\n"
                                     "Taille du fichier: %3 KB")
                                 .arg(fileName)
                                 .arg(total)
                                 .arg(QFileInfo(fileName).size() / 1024));

}

// ==================== PREDICTION DETAILS ====================

void WasteRecycling::on_GCont_predictionButton_clicked()
{
    int id = getSelectedId();
    if (id == -1) {
        QMessageBox::warning(this, "Selection", "Veuillez selectionner un conteneur.");
        return;
    }

    Conteneur conteneur = Conteneur::getConteneurById(id);

    if (conteneur.getId() != 0) {
        showPredictionDetails(conteneur);
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de recuperer les donnees du conteneur.");
    }
}

void WasteRecycling::showPredictionDetails(const Conteneur& conteneur)
{
    // Calculate prediction data
    int fillPercent = conteneur.getPourcentageRemplissage();
    QString alertLevel = conteneur.getNiveauAlerte();
    double fillRatePerDay = conteneur.getFillRatePerDay();
    int daysUntilFull = conteneur.getDaysUntilFull();
    QString timeUntilFull = conteneur.getTimeUntilFull();

    // Calculate estimated full date
    QDate estimatedFullDate;
    QString estimatedFullDateStr = "Non disponible";

    if (daysUntilFull >= 0) {
        estimatedFullDate = QDate::currentDate().addDays(daysUntilFull);
        estimatedFullDateStr = estimatedFullDate.toString("dddd dd MMMM yyyy");
    }

    // Calculate remaining capacity
    int remainingCapacity = conteneur.getCapaciteMax() - conteneur.getQuantiteActuelle();

    // Calculate daily fill rate in percentage
    double dailyFillPercent = (fillRatePerDay / conteneur.getCapaciteMax()) * 100;

    // Create detailed message
    QString details;
    details += "=== DETAILS DE PREDICTION ===\n\n";
    details += "Conteneur ID: " + QString::number(conteneur.getId()) + "\n";
    details += "Type: " + conteneur.getType() + "\n\n";

    details += "--- ETAT ACTUEL ---\n";
    details += "Capacite maximale: " + QString::number(conteneur.getCapaciteMax()) + " L\n";
    details += "Quantite actuelle: " + QString::number(conteneur.getQuantiteActuelle()) + " L\n";
    details += "Remplissage: " + QString::number(fillPercent) + "%\n";
    details += "Niveau d'alerte: " + alertLevel + "\n";
    details += "Derniere mise a jour: " + conteneur.getDateMaj().toString("dd/MM/yyyy") + "\n\n";

    details += "--- ANALYSE ---\n";
    details += "Capacite restante: " + QString::number(remainingCapacity) + " L\n";

    if (fillRatePerDay > 0) {
        details += "Taux de remplissage: " + QString::number(fillRatePerDay, 'f', 1) + " L/jour\n";
        details += "Taux de remplissage: " + QString::number(dailyFillPercent, 'f', 1) + "%/jour\n";
    } else {
        details += "Taux de remplissage: Aucune activite detectee\n";
    }

    details += "\n--- PREDICTION ---\n";

    if (daysUntilFull < 0) {
        details += "Statut: Aucune activite - prediction non disponible\n";
        details += "Recommandation: Surveiller le conteneur pour detecter une activite\n";
    } else if (daysUntilFull == 0) {
        details += "Statut: Conteneur DEJA PLEIN!\n";
        details += "Action URGENTE: Une collecte est necessaire immediatement!\n";
    } else {
        details += "Temps estime avant d'etre plein: " + timeUntilFull + "\n";
        details += "Date estimee d'atteinte de capacite: " + estimatedFullDateStr + "\n";

        if (daysUntilFull <= 2) {
            details += "\n⚠️ URGENT: Le conteneur sera plein tres prochainement!\n";
            details += "Action recommandee: Planifier une collecte dans les 24-48 heures.\n";
        } else if (daysUntilFull <= 5) {
            details += "\n⚠️ ATTENTION: Le conteneur sera plein cette semaine!\n";
            details += "Action recommandee: Planifier une collecte dans les prochains jours.\n";
        } else {
            details += "\n✅ OK: Le conteneur a encore de la capacite.\n";
            details += "Action recommandee: Surveillance normale.\n";
        }
    }

    details += "\n--- RECOMMANDATIONS ---\n";

    if (alertLevel == "Critique") {
        details += "• Niveau CRITIQUE - Intervention immediate requise!\n";
        details += "• Contacter l'equipe de collecte d'urgence.\n";
    } else if (alertLevel == "Avertissement") {
        details += "• Niveau d'alerte atteint.\n";
        details += "• Planifier une collecte prochainement.\n";
    } else {
        details += "• Situation normale.\n";
        details += "• Surveillance reguliere recommandee.\n";
    }

    if (fillRatePerDay > 100) {
        details += "• Taux de remplissage eleve - verifier la source des dechets.\n";
    } else if (fillRatePerDay < 10 && fillRatePerDay > 0) {
        details += "• Taux de remplissage faible - utilisation normale.\n";
    }

    details += "\n--- INFORMATION SUPPLEMENTAIRE ---\n";
    details += "Cette prediction est basee sur:\n";
    details += "• Le taux de remplissage actuel\n";
    details += "• La date de derniere mise a jour\n";
    if (conteneur.getDateMaj() == QDate::currentDate()) {
        details += "• Aucun historique - utilisation des valeurs par defaut par type de conteneur\n";
    } else {
        int daysSinceUpdate = conteneur.getDateMaj().daysTo(QDate::currentDate());
        details += "• Periode analysee: " + QString::number(daysSinceUpdate) + " jours\n";
    }

    details += "\nCordialement,\n";
    details += "Systeme de Gestion des Conteneurs - Waste Manager Pro";

    // Show the prediction in a message box
    QMessageBox msgBox;
    msgBox.setWindowTitle("Prediction - Conteneur " + QString::number(conteneur.getId()));
    msgBox.setText(details);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::Ok);

    // Set a larger minimum size for the message box
    msgBox.setMinimumSize(500, 600);

    msgBox.exec();
}

// ==================== STATISTICS ====================

void WasteRecycling::on_GCont_statisticsButton_clicked()
{
    showStatisticsDialog();
}

void WasteRecycling::showStatisticsDialog()
{
    // Get statistics from the database
    Conteneur::ContainerStats stats = Conteneur::getStatistics();

    // Create a custom dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Statistiques des Conteneurs");
    dialog.setMinimumSize(550, 500);
    dialog.setModal(true);

    // Set dark background for the dialog
    dialog.setStyleSheet("QDialog { background-color: #2C3E50; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);

    // Title
    QLabel* titleLabel = new QLabel("STATISTIQUES DES CONTENEURS", &dialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFFFFF; padding: 15px; background-color: #245F52; border-radius: 10px;");
    mainLayout->addWidget(titleLabel);

    // Total containers
    QLabel* totalLabel = new QLabel(QString("Nombre total de conteneurs: %1").arg(stats.total), &dialog);
    totalLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF; padding: 10px; background-color: #34495E; border-radius: 8px;");
    totalLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(totalLabel);

    // Separator line
    QFrame* line = new QFrame(&dialog);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #FFFFFF; background-color: #7F8C8D;");
    mainLayout->addWidget(line);

    // Create a grid layout for statistics
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setVerticalSpacing(15);
    gridLayout->setHorizontalSpacing(10);

    // Define categories and their colors
    struct StatItem {
        QString name;
        int count;
        double percent;
        QString color;
        QString icon;
    };

    QList<StatItem> items;
    items.append({"Vides (0%)", stats.empty, stats.emptyPercent, "#95A5A6", "⬜"});
    items.append({"Peu remplis (1-25%)", stats.quarterFull, stats.quarterFullPercent, "#3498DB", "🔵"});
    items.append({"Mi-pleins (26-50%)", stats.halfFull, stats.halfFullPercent, "#2ECC71", "🟢"});
    items.append({"Bien remplis (51-75%)", stats.threeQuarterFull, stats.threeQuarterFullPercent, "#F39C12", "🟠"});
    items.append({"Presque pleins (76-99%)", stats.almostFull, stats.almostFullPercent, "#E67E22", "🟡"});
    items.append({"Pleins (100%)", stats.full, stats.fullPercent, "#E74C3C", "🔴"});

    int row = 0;
    for (const StatItem& item : items) {
        // Icon
        QLabel* iconLabel = new QLabel(item.icon, &dialog);
        iconLabel->setStyleSheet("font-size: 24px; background-color: transparent;");
        iconLabel->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(iconLabel, row, 0);

        // Category name
        QLabel* nameLabel = new QLabel(item.name, &dialog);
        nameLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #FFFFFF; background-color: transparent;");
        gridLayout->addWidget(nameLabel, row, 1);

        // Count
        QLabel* countLabel = new QLabel(QString::number(item.count), &dialog);
        countLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; background-color: transparent;").arg(item.color));
        countLabel->setAlignment(Qt::AlignRight);
        gridLayout->addWidget(countLabel, row, 2);

        // Progress bar background
        QProgressBar* progressBar = new QProgressBar(&dialog);
        progressBar->setRange(0, 100);
        progressBar->setValue((int)item.percent);
        progressBar->setTextVisible(true);
        progressBar->setFormat(QString::number(item.percent, 'f', 1) + "%");
        progressBar->setStyleSheet(QString(
                                       "QProgressBar { border: 2px solid #555; border-radius: 8px; text-align: center; background-color: #555; color: #FFFFFF; font-weight: bold; }"
                                       "QProgressBar::chunk { background-color: %1; border-radius: 6px; }"
                                       ).arg(item.color));
        progressBar->setFixedHeight(25);
        gridLayout->addWidget(progressBar, row, 3);

        row++;
    }

    mainLayout->addLayout(gridLayout);

    // Separator line
    QFrame* line2 = new QFrame(&dialog);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setStyleSheet("color: #FFFFFF; background-color: #7F8C8D;");
    mainLayout->addWidget(line2);

    // Summary section
    QLabel* summaryTitle = new QLabel("RESUME", &dialog);
    summaryTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #F39C12; padding: 8px; background-color: transparent;");
    summaryTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(summaryTitle);

    QGridLayout* summaryGrid = new QGridLayout();
    summaryGrid->setVerticalSpacing(10);

    // Calculate some additional stats
    int containersNeedingAttention = stats.almostFull + stats.full;
    double containersNeedingAttentionPercent = (stats.total > 0) ? (double)containersNeedingAttention / stats.total * 100 : 0;

    int containersHealthy = stats.empty + stats.quarterFull + stats.halfFull;
    double containersHealthyPercent = (stats.total > 0) ? (double)containersHealthy / stats.total * 100 : 0;

    QLabel* healthyLabel = new QLabel("Conteneurs sains:", &dialog);
    healthyLabel->setStyleSheet("font-weight: bold; color: #FFFFFF; background-color: transparent; font-size: 14px;");
    summaryGrid->addWidget(healthyLabel, 0, 0);

    QLabel* healthyValue = new QLabel(QString("%1 (%2%)").arg(containersHealthy).arg(containersHealthyPercent, 0, 'f', 1), &dialog);
    healthyValue->setStyleSheet("color: #2ECC71; font-weight: bold; background-color: transparent; font-size: 14px;");
    summaryGrid->addWidget(healthyValue, 0, 1);

    QLabel* attentionLabel = new QLabel("Conteneurs necessitant attention:", &dialog);
    attentionLabel->setStyleSheet("font-weight: bold; color: #FFFFFF; background-color: transparent; font-size: 14px;");
    summaryGrid->addWidget(attentionLabel, 1, 0);

    QLabel* attentionValue = new QLabel(QString("%1 (%2%)").arg(containersNeedingAttention).arg(containersNeedingAttentionPercent, 0, 'f', 1), &dialog);
    attentionValue->setStyleSheet("color: #E74C3C; font-weight: bold; background-color: transparent; font-size: 14px;");
    summaryGrid->addWidget(attentionValue, 1, 1);

    mainLayout->addLayout(summaryGrid);

    // Recommendation based on statistics
    QLabel* recommendation = new QLabel(&dialog);
    recommendation->setWordWrap(true);
    recommendation->setAlignment(Qt::AlignCenter);
    recommendation->setStyleSheet("background-color: #34495E; padding: 12px; border-radius: 10px; margin-top: 10px; color: #FFFFFF; font-size: 13px;");

    if (stats.fullPercent > 20) {
        recommendation->setText("⚠️ RECOMMANDATION: Plus de 20% des conteneurs sont pleins. Une collecte urgente est recommandee.");
    } else if (stats.almostFullPercent > 30) {
        recommendation->setText("⚠️ RECOMMANDATION: Plus de 30% des conteneurs sont presque pleins. Planifier une collecte prochainement.");
    } else if (stats.emptyPercent > 40) {
        recommendation->setText("ℹ️ INFORMATION: Plus de 40% des conteneurs sont vides. La capacite est largement disponible.");
    } else {
        recommendation->setText("✅ SITUATION NORMALE: La repartition des conteneurs est equilibree. Surveillance reguliere recommandee.");
    }

    mainLayout->addWidget(recommendation);

    // Close button
    QPushButton* closeButton = new QPushButton("Fermer", &dialog);
    closeButton->setStyleSheet("background-color: #245F52; color: white; padding: 12px; border-radius: 10px; font-weight: bold; font-size: 14px; min-width: 120px;");
    closeButton->setCursor(Qt::PointingHandCursor);
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    dialog.exec();
}

// ==================== ARDUINO INTEGRATION ====================

void WasteRecycling::setupArduino()
{
    // Create status label for Arduino
    arduinoStatusLabel = new QLabel(this);
    arduinoStatusLabel->setText(" Arduino: Disconnected ");
    arduinoStatusLabel->setStyleSheet("QLabel { background-color: #E74C3C; color: white; padding: 5px; border-radius: 5px; font-weight: bold; }");

    // Setup reconnect timer
    arduinoReconnectTimer->setInterval(5000); // Try every 5 seconds
    arduinoReconnectTimer->setSingleShot(false);
    connect(arduinoReconnectTimer, &QTimer::timeout, this, &WasteRecycling::attemptArduinoReconnection);

    // Connect serial port signals
    connect(arduinoSerial, &QSerialPort::readyRead, this, &WasteRecycling::onArduinoDataReceived);
    connect(arduinoSerial, &QSerialPort::errorOccurred, this, &WasteRecycling::onArduinoError);

    // Try to connect to Arduino
    scanForArduinoPort();
}

void WasteRecycling::scanForArduinoPort()
{
    qDebug() << "Scanning for Arduino...";

    // List all available serial ports
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &port : ports) {
        qDebug() << "Found port:" << port.portName()
        << "Description:" << port.description()
        << "Manufacturer:" << port.manufacturer();

        // Try to connect to potential Arduino port
        arduinoSerial->setPortName(port.portName());
        arduinoSerial->setBaudRate(QSerialPort::Baud9600);
        arduinoSerial->setDataBits(QSerialPort::Data8);
        arduinoSerial->setParity(QSerialPort::NoParity);
        arduinoSerial->setStopBits(QSerialPort::OneStop);
        arduinoSerial->setFlowControl(QSerialPort::NoFlowControl);

        if (arduinoSerial->open(QIODevice::ReadWrite)) {
            arduinoConnected = true;
            arduinoReconnectTimer->stop();
            updateArduinoStatus(true);
            qDebug() << "Successfully connected to Arduino on port:" << port.portName();
            return;
        }
    }

    // No Arduino found
    arduinoConnected = false;
    updateArduinoStatus(false);
    qDebug() << "No Arduino found. Will retry...";
    arduinoReconnectTimer->start();
}

void WasteRecycling::connectToArduino()
{
    if (arduinoSerial->isOpen()) {
        return; // Already connected
    }

    scanForArduinoPort();
}

void WasteRecycling::disconnectFromArduino()
{
    if (arduinoSerial->isOpen()) {
        arduinoSerial->close();
    }
    arduinoConnected = false;
    arduinoReconnectTimer->stop();
    updateArduinoStatus(false);
    qDebug() << "Disconnected from Arduino";
}

void WasteRecycling::attemptArduinoReconnection()
{
    if (!arduinoConnected && !arduinoSerial->isOpen()) {
        qDebug() << "Attempting to reconnect to Arduino...";
        scanForArduinoPort();
    }
}

void WasteRecycling::updateArduinoStatus(bool connected)
{
    if (connected) {
        arduinoStatusLabel->setText(" Arduino: Connected ✓ ");
        arduinoStatusLabel->setStyleSheet("QLabel { background-color: #27AE60; color: white; padding: 5px; border-radius: 5px; font-weight: bold; }");
    } else {
        arduinoStatusLabel->setText(" Arduino: Disconnected ✗ ");
        arduinoStatusLabel->setStyleSheet("QLabel { background-color: #E74C3C; color: white; padding: 5px; border-radius: 5px; font-weight: bold; }");
    }
}



void WasteRecycling::onArduinoDataReceived()
{
    if (arduinoSerial->canReadLine()) {
        QByteArray data = arduinoSerial->readLine();
        QString response = QString::fromUtf8(data).trimmed();
        qDebug() << "Received from Arduino:" << response;

    }
}

void WasteRecycling::onArduinoError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        qDebug() << "Serial port error:" << arduinoSerial->errorString();
        arduinoConnected = false;
        updateArduinoStatus(false);

        // Try to reconnect
        arduinoSerial->close();
        arduinoReconnectTimer->start();
    }
}


// ********************************* GESTION CONTENEURS *****************************************



// ********************************* PAGES MACHINES *******************************************

void WasteRecycling::on_MRbtn_machine_2_clicked()
{
    ui->MRstackedWidget->setCurrentIndex(0);
}


void WasteRecycling::on_MRbtn_recyclage_2_clicked()
{
    ui->MRstackedWidget->setCurrentIndex(1);
}


void WasteRecycling::on_MRbtn_machine_clicked()
{
    ui->MRstackedWidget->setCurrentIndex(0);
}


void WasteRecycling::on_MRbtn_recyclage_clicked()
{
    ui->MRstackedWidget->setCurrentIndex(1);
}
// ********************************* Pages MACHINES *******************************************


void WasteRecycling::on_MRbtn_lancer_clicked()
{
    QString mode = ui->MRbtn_lancer->property("mode").toString();

    // Récupération des valeurs du formulaire
    QString typeDechet = ui->MRcombo_typeDechet->currentText();
    int capacite = ui->MRspinBox_capacite->value();
    QString nomMachine = ui->MRlineEdit_nomMachine->text().trimmed();

    // Contrôle de saisie
    if (nomMachine.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez saisir un nom de machine !");
        ui->MRlineEdit_nomMachine->setFocus();
        return;
    }

    if (nomMachine.length() > 30) {
        QMessageBox::warning(this, "Erreur", "Le nom de la machine ne doit pas dépasser 30 caractères !");
        ui->MRlineEdit_nomMachine->setFocus();
        return;
    }

    if (capacite <= 0) {
        QMessageBox::warning(this, "Erreur", "La capacité doit être supérieure à 0 !");
        ui->MRspinBox_capacite->setFocus();
        return;
    }

    if (mode == "modifier")
    {
        int id = ui->MRbtn_lancer->property("id_modif").toInt();
        if (id <= 0) {
            QMessageBox::warning(this, "Erreur", "ID invalide pour la modification");
            return;
        }

        Machine m;
        m.setTypeDechet(typeDechet);
        m.setCapacite(capacite);
        m.setNomMachine(nomMachine);

        if (m.modifier(id))
        {
            QMessageBox::information(this, "Succès", "La machine a été modifiée");

            // Rafraîchir le tableau
            QAbstractItemModel *oldModel = ui->MRtable->model();
            ui->MRtable->setModel(mach.afficher());
            if (oldModel && oldModel != ui->MRtable->model()) {
                delete oldModel;
            }

            // Reset du bouton
            ui->MRbtn_lancer->setText("Ajouter");
            ui->MRbtn_lancer->setProperty("mode", "ajout");
            ui->MRbtn_lancer->setProperty("id_modif", 0);

            // Vider les champs
            ui->MRcombo_typeDechet->setCurrentIndex(0);
            ui->MRspinBox_capacite->setValue(0);
            ui->MRlineEdit_nomMachine->clear();
        }
        else
        {
            QMessageBox::warning(this, "Erreur", "Échec de la modification");
        }
    }
    else  // mode ajout
    {
        Machine m;
        m.setTypeDechet(typeDechet);
        m.setCapacite(capacite);
        m.setNomMachine(nomMachine);
        // La date d'ajout est automatique

        if (m.ajouter())
        {
            // Rafraîchir le tableau
            QAbstractItemModel *oldModel = ui->MRtable->model();
            ui->MRtable->setModel(mach.afficher());
            if (oldModel && oldModel != ui->MRtable->model()) {
                delete oldModel;
            }

            ui->MRtable->scrollToBottom();

            // Vider les champs
            ui->MRcombo_typeDechet->setCurrentIndex(0);
            ui->MRspinBox_capacite->setValue(0);
            ui->MRlineEdit_nomMachine->clear();

            QMessageBox::information(this, "Succès", "Machine ajoutée avec succès !");
        }
        else
        {
            QMessageBox::critical(this, "Erreur", "Échec de l'ajout");
        }
    }
}

void WasteRecycling::on_MRbtn_supprimer_clicked()
{
    QModelIndexList lignes = ui->MRtable->selectionModel()->selectedRows();

    if (lignes.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une machine à supprimer");
        return;
    }

    int ligne = lignes.first().row();
    int id = ui->MRtable->model()->data(ui->MRtable->model()->index(ligne, 0)).toInt();

    // Confirmation
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation", "Êtes-vous sûr de vouloir supprimer cette machine ?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }
    Machine m;
    if (m.supprimer(id)) {
        QAbstractItemModel *oldModel = ui->MRtable->model();
        ui->MRtable->setModel(mach.afficher());
        if (oldModel && oldModel != ui->MRtable->model()) {
            delete oldModel;
        }
        QMessageBox::information(this, "Succès", "Machine supprimée avec succès");
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression");
    }
}

void WasteRecycling::on_MRtable_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
    QAbstractItemModel *model = ui->MRtable->model();

    int id = model->data(model->index(row, 0)).toInt();
    QString typeDechet = model->data(model->index(row, 1)).toString();
    QString nomMachine = model->data(model->index(row, 2)).toString();
    int capacite = model->data(model->index(row, 3)).toInt();

    // Remplir les champs
    ui->MRcombo_typeDechet->setCurrentText(typeDechet);
    ui->MRlineEdit_nomMachine->setText(nomMachine);
    ui->MRspinBox_capacite->setValue(capacite);

    // Passer en mode modification
    ui->MRbtn_lancer->setText("Modifier");
    ui->MRbtn_lancer->setProperty("mode", "modifier");
    ui->MRbtn_lancer->setProperty("id_modif", id);

    ui->MRlineEdit_nomMachine->setFocus();
}

void WasteRecycling::on_MRtable_clicked(const QModelIndex &index)
{
    QString mode = ui->MRbtn_lancer->property("mode").toString();

    if (mode == "modifier")
    {
        int idClique = ui->MRtable->model()->data(ui->MRtable->model()->index(index.row(), 0)).toInt();
        int idEnCours = ui->MRbtn_lancer->property("id_modif").toInt();

        if (idClique != idEnCours)
        {
            // Reset du mode modification
            ui->MRbtn_lancer->setText("Ajouter");
            ui->MRbtn_lancer->setProperty("mode", "ajout");
            ui->MRbtn_lancer->setProperty("id_modif", 0);

            ui->MRcombo_typeDechet->setCurrentIndex(0);
            ui->MRspinBox_capacite->setValue(0);
            ui->MRlineEdit_nomMachine->clear();
        }
    }
}


// --------------------------------- GESTION MACHINES -------------------------------------------


// --------------------------------- GESTION Recyclage -------------------------------------------


void WasteRecycling::on_MRbtn_lancer_2_clicked() {

    // --- NOUVELLE PARTIE : GESTION DU MODE CAPTEUR ---
    if (ui->MRbtn_lancer_2->text() == "Ajouter la sortie") {
        // On utilise la méthode métier de la classe Recyclage (sans SQL ici)
        bool ok = recy.terminerRecyclage(modif.id_c_source,
                                         modif.id_m_cible,
                                         modif.date_debut,
                                         this->poidsDernierRecu,
                                         modif.quantite_entree);

        if (ok) {
            QMessageBox::information(this, "Succès", "Sortie enregistrée et recyclage terminé !");
            ui->MRtable_2->setModel(recy.afficherHistorique());

            // On remet l'UI en mode normal
            ui->MRbtn_lancer_2->setText("Lancer");
            ui->MRlabel_QteSortie->setText("Nulle");
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la mise à jour du recyclage.");
        }
        return; // On arrête la fonction ici pour ne pas exécuter le code d'ajout ci-dessous
    }

    // --- TON CODE D'ORIGINE (MODE LANCER / AJOUT) ---
    int id_c = ui->MRlineEdit_IdCont->text().toInt();
    int id_m = ui->MRlineEdit_IdMach->text().toInt();
    float qte = ui->MRspinBox_Quantite->value();

    int res = recy.verifierTout(id_c, id_m, qte);

    if (res == 1) {
        recy = Recyclage(id_c, id_m, qte);

        if(recy.ajouterRecyclage()){
            ui->MRtable_2->setModel(recy.afficherHistorique());
            QMessageBox::information(this, "Succès", "Recyclage ajouté !");
        } else {
            QSqlError err = QSqlDatabase::database().lastError();
            QMessageBox::critical(this, "Erreur Oracle", "Impossible d'ajouter : " + err.text());
        }
    }
    else {
        QString msg;
        if (res == 0)  msg = "L'ID Conteneur n'existe pas.";
        if (res == -1) msg = "L'ID Machine n'existe pas.";
        if (res == 2)  msg = "Incompatibilité : Les types de déchets ne correspondent pas !";
        if (res == 3)  msg = "Capacité dépassée : La machine est trop petite pour cette quantité.";
        if (res == 4)  msg = "Stock insuffisant : Le conteneur ne contient pas assez de déchets.";

        QMessageBox::warning(this, "Erreur de saisie", msg);
    }
}

void WasteRecycling::on_MRbtn_supprimer_2_clicked()
{
    // On récupère l'index de la cellule actuellement cliquée/sélectionnée
    QModelIndex currentIndex = ui->MRtable_2->currentIndex();

    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un enregistrement de recyclage à supprimer");
        return;
    }

    // On récupère le numéro de la ligne à partir de la cellule
    int ligne = currentIndex.row();
    QAbstractItemModel *model = ui->MRtable_2->model();

    // Le reste du code reste identique
    int id_c = model->data(model->index(ligne, 0)).toInt();
    int id_m = model->data(model->index(ligne, 1)).toInt();
    QString date_d = model->data(model->index(ligne, 4)).toString();
    // 3. Confirmation avec QMessageBox
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
                                                              "Êtes-vous sûr de vouloir supprimer cet historique de recyclage ?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    // 4. Exécution de la suppression via la classe Recyclage
    if (recy.supprimerRecyclage(id_c, id_m, date_d)) {
        // 5. Mise à jour du modèle et gestion de la mémoire
        QAbstractItemModel *oldModel = ui->MRtable_2->model();
        ui->MRtable_2->setModel(recy.afficherHistorique());

        if (oldModel && oldModel != ui->MRtable_2->model()) {
            delete oldModel;
        }

        // 6. Message de succès
        QMessageBox::information(this, "Succès", "Recyclage supprimé avec succès");

        // Optionnel : Reset de l'UI
        ui->MRbtn_lancer_2->setText("Lancer");
        ui->MRlabel_QteSortie->setText("Nulle");

    } else {
        // 7. Message d'erreur
        QMessageBox::critical(this, "Erreur", "Échec de la suppression. Vérifiez la correspondance de la date.");
    }
}

void WasteRecycling::on_MRtable_2_clicked(const QModelIndex &index)
{
    int row = index.row();
    QAbstractItemModel *model = ui->MRtable_2->model();

    // Récupération des données pour l'affichage
    ui->MRlineEdit_IdCont->setText(model->index(row, 0).data().toString());
    ui->MRlineEdit_IdMach->setText(model->index(row, 1).data().toString());
    ui->MRspinBox_Quantite->setValue(model->index(row, 2).data().toDouble());

    // RÉCUPÉRATION DE L'ÉTAT (Index 3)
    QString etat = model->index(row, 3).data().toString().trimmed();

    // DEBUG : Regarde dans la console en bas de Qt Creator ce qui s'affiche !
    qDebug() << "Ligne cliquée :" << row << "| État récupéré :" << etat;

    // COMPARAISON ROBUSTE (On cherche "En cours" sans se soucier des points ou de la casse)
    if (etat.contains("En cours", Qt::CaseInsensitive))
    {
        // On remplit le struct modif pour préparer l'UPDATE
        modif.id_c_source = model->index(row, 0).data().toInt();
        modif.id_m_cible = model->index(row, 1).data().toInt();
        modif.quantite_entree = model->index(row, 2).data().toFloat();
        modif.date_debut = model->index(row, 4).data().toString(); // La date en colonne 4

        // MISE À JOUR UI
        ui->MRlabel_QteSortie->setText(QString::number(this->poidsDernierRecu, 'f', 2) + " kg");
        ui->MRbtn_lancer_2->setText("Ajouter la sortie");
    }
    else
    {
        // Si c'est "Terminé" ou autre chose, on revient au mode normal
        ui->MRbtn_lancer_2->setText("Lancer");
        ui->MRlabel_QteSortie->setText("Nulle");
    }
}

void WasteRecycling::mousePressEvent(QMouseEvent *event)
{
    // On vérifie si le clic a eu lieu sur le fond de la fenêtre
    // et non sur un bouton ou un tableau
    if (ui->MRtable_2->underMouse() == false)
    {
        // 1. On désélectionne la ligne du tableau
        ui->MRtable_2->clearSelection();
        ui->MRtable_2->setCurrentIndex(QModelIndex());

        // 2. On remet le bouton en mode normal
        ui->MRbtn_lancer_2->setText("Lancer");
        ui->MRlabel_QteSortie->setText("Nulle");

        // 3. (Facultatif) On vide les LineEdits
        ui->MRlineEdit_IdCont->clear();
        ui->MRlineEdit_IdMach->clear();
        ui->MRspinBox_Quantite->setValue(0);

        qDebug() << "Clic hors tableau : Mode 'Lancer' restauré.";
    }

    // On laisse l'événement se propager normalement pour les autres widgets
    QMainWindow::mousePressEvent(event);
}

void WasteRecycling::lireDonneesCapteur(QByteArray buffer) {

    // L'Arduino envoie un '\n' (retour à la ligne) à la fin de chaque Serial.println
    /*
    if (buffer.contains('\n')) {
        qDebug() << "[Arduino] Reçu :" << buffer;
        // On sépare la donnée complète
        QList<QByteArray> lines = buffer.split('\n');

        // On prend la ligne la plus récente (celle juste avant le dernier \n)
        QByteArray dataComplete = lines.at(lines.size() - 2).trimmed();
    */
    QByteArray dataComplete = buffer;
        if (!dataComplete.isEmpty()) {
            bool ok;
            float valeur = dataComplete.toFloat(&ok);

            if (ok) {
                this->poidsDernierRecu = valeur;
                ui->MRlabel_QteSortie->setText(QString::number(valeur, 'f', 2) + " kg");
            }
        }
        // On vide le buffer pour la suite
        buffer.clear();
    //}
}

// --------------------------------- GESTION Recyclage -------------------------------------------


// ==================== METIERS MACHINES ====================

void WasteRecycling::on_MRbtn_stats_clicked()
{
    // Créer une boîte de dialogue popup
    QDialog *statsDialog = new QDialog(this);
    statsDialog->setWindowTitle("📊 Statistiques des Machines");
    statsDialog->setFixedSize(700, 550);
    statsDialog->setStyleSheet("background-color: #ECF5E9;");
    statsDialog->setModal(true);
    statsDialog->setAttribute(Qt::WA_DeleteOnClose);

    // Layout principal
    QVBoxLayout *mainLayout = new QVBoxLayout(statsDialog);

    // Titre
    QLabel *titleLabel = new QLabel("Répartition des machines par type de déchet");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #25665A; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    // Récupérer les données
    QMap<QString, int> data = mach.getStatsParType();

    int total = 0;
    for (auto it = data.begin(); it != data.end(); ++it)
        total += it.value();
    QPieSeries *series = new QPieSeries();
    // Ajouter les parts
    for (auto it = data.begin(); it != data.end(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        double pourcentage = (it.value() * 100.0) / total;
        slice->setLabel(QString("%1\n%2%").arg(it.key()).arg(pourcentage, 0, 'f', 1));
        slice->setLabelVisible(true);

        if (it.key() == "Plastique") slice->setColor(QColor(52, 152, 219));
        else if (it.key() == "Verre") slice->setColor(QColor(46, 204, 113));
        else if (it.key() == "Métal") slice->setColor(QColor(241, 196, 15));
        else slice->setColor(QColor(155, 89, 182));
    }

    // Créer le graphique
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setBackgroundVisible(false);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setFixedHeight(350);

    // Zone des détails
    QFrame *detailsFrame = new QFrame();
    detailsFrame->setStyleSheet("background-color: white; border-radius: 15px;");
    detailsFrame->setFixedHeight(120);

    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsFrame);

    // Calculer les stats
    QMap<QString, double> caps = mach.getStatsCapacite();
    int capTotale = caps["total"];
    double capMoyenne = caps["moyenne"];

    // Texte des détails - Correction du warning multi-arg
    QString detailsText = QString(
                              "<b>📊 Résumé :</b><br>"
                              "• <b>Nombre total :</b> %1 machine(s)<br>"
                              "• <b>Capacité totale :</b> %2 kg<br>"
                              "• <b>Capacité moyenne :</b> %3 kg<br><br>"
                              "<b>📈 Répartition :</b><br>"
                              ).arg(total)
                              .arg(capTotale)
                              .arg(QString::number(capMoyenne, 'f', 0));

    for (auto it = data.begin(); it != data.end(); ++it) {
        double pct = (it.value() * 100.0) / total;
        detailsText += QString("• <b>%1 :</b> %2 machine(s) (%3%)<br>")
                           .arg(it.key())
                           .arg(it.value())
                           .arg(pct, 0, 'f', 1);
    }

    QLabel *detailsLabel = new QLabel(detailsText);
    detailsLabel->setStyleSheet("color: #25665A; font-size: 12px; padding: 10px;");
    detailsLabel->setWordWrap(true);
    detailsLayout->addWidget(detailsLabel);

    // Bouton Fermer
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setFixedSize(100, 35);
    closeBtn->setStyleSheet("background-color: #25665A; color: white; border-radius: 8px; font-weight: bold;");

    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    buttonLayout->addStretch();

    // Ajouter tout au layout principal
    mainLayout->addWidget(chartView);
    mainLayout->addWidget(detailsFrame);
    mainLayout->addLayout(buttonLayout);

    // Fermer
    connect(closeBtn, &QPushButton::clicked, statsDialog, &QDialog::close);

    // Afficher
    statsDialog->exec();
}

void WasteRecycling::on_MRline_recherche_returnPressed()
{
    // Quand on appuie sur Entrée dans le champ de recherche
    QString recherche = ui->MRline_recherche->text().trimmed();

    if (recherche.isEmpty()) {
        // Si vide, afficher toutes les machines
        ui->MRtable->setModel(mach.afficher());
    } else {
        // Sinon, afficher les résultats de la recherche
        ui->MRtable->setModel(mach.rechercher(recherche));
    }
}

void WasteRecycling::on_MRbtn_trier_clicked()
{
    // Trier par ordre alphabétique des types
    ui->MRtable->setModel(mach.trierParType("Tous"));
}
void WasteRecycling::on_MRbtn_export_clicked()
{
    QString chemin = QFileDialog::getSaveFileName(this, "Exporter en PDF", "machines.pdf", "PDF (*.pdf)");
    if (chemin.isEmpty()) return;

    if (mach.exporterPDF(chemin))
        QMessageBox::information(this, "Succès", "PDF exporté avec succès !");
    else
        QMessageBox::critical(this, "Erreur", "Échec de l'export PDF.");
}
// ==================== METIERS MACHINES ====================


//////////////////////////////////////////////////////////////////////////////////////////GPC/////////////////////////////////////////////////////////////////////////////////
////////////////AJOUTER GPC

void WasteRecycling::on_GPC_BtnSave_clicked()
{
    QString zone = ui->GPC_FormCmbZone->currentText();
    QString adresse = ui->GPC_FormTxtAddress->text();
    int niveau = ui->GPC_FormLevel->value();

    // date en format Oracle-safe
    QString date = ui->GPC_FormDate->date().toString("yyyy-MM-dd");

    QString typeZone = ui->GPC_FormCmbPriority->currentText();

    // Ici : si tu gardes le constructeur avec id, mets id=0 (non utilisé)
    PointCollecte pc(0, zone, adresse, niveau, date, typeZone);

    if (pc.ajouter())
    {
        ui->gpcTableView->setModel(PC.afficher());
        ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));
        QMessageBox::information(this, "GPC", "Ajout réussi !");

        ui->GPC_FormCmbZone->setCurrentIndex(0);
        ui->GPC_FormTxtAddress->clear();
        ui->GPC_FormLevel->setValue(0);
        ui->GPC_FormDate->setDate(QDate::currentDate());
        ui->GPC_FormCmbPriority->setCurrentIndex(0);
        //new
        resetPredictionFields();
        gpcSelectedId = -1;
        ui->GPC_BtnPredict->setEnabled(false);

        // Infiltration Camion
        adresse += ", Délégation Raoued, Gouvernorat Ariana, Tunisie";
        CamTemp.camConvSauvCoord(adresse);
        // 😈😈
    }else
    {
        QMessageBox::critical(this, "GPC", "Erreur ajout !");
    }
}

/////
ProgressBarDelegate::ProgressBarDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QColor ProgressBarDelegate::getFillColor(int niveau) const
{
    if (niveau < 50)
        return QColor(76, 175, 80);   // vert
    if (niveau < 80)
        return QColor(255, 152, 0);   // orange
    return QColor(244, 67, 54);       // rouge
}

void ProgressBarDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    int value = index.data().toInt();

    painter->save();

    QRect rect = option.rect.adjusted(4, 8, -4, -8);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(217, 217, 217));
    painter->drawRoundedRect(rect, 6, 6);

    int progressWidth = rect.width() * value / 100;
    QRect progressRect(rect.left(), rect.top(), progressWidth, rect.height());

    painter->setBrush(getFillColor(value));
    painter->drawRoundedRect(progressRect, 6, 6);

    painter->setPen(Qt::black);
    painter->drawText(option.rect, Qt::AlignCenter, QString::number(value) + "%");

    painter->restore();
}
////////////////SUPPRIMER GPC
void WasteRecycling::on_gpcBtnSupprimer_clicked()
{
    QModelIndex index = ui->gpcTableView->currentIndex();
    if(!index.isValid())
    {
        QMessageBox::warning(this, "GPC", "Sélectionne une ligne à supprimer.");
        return;
    }

    int row = index.row();
    int id = ui->gpcTableView->model()->index(row, 0).data().toInt(); // col 0 = ID_PC

    if (PC.supprimer(id))
    {
        ui->gpcTableView->setModel(PC.afficher());
        ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));
        QMessageBox::information(this, "GPC", "Suppression réussie !");
    }
    else
    {
        QMessageBox::critical(this, "GPC", "Erreur suppression !");
    }
}
////////////////MODIFIER GPC
void WasteRecycling::on_gpcTableView_doubleClicked(const QModelIndex &index)
{
    int row = index.row();

    gpcSelectedId = ui->gpcTableView->model()->index(row, 0).data().toInt();
    //new
    // ui->GPC_BtnPredict->setEnabled(true);

    QString zone = ui->gpcTableView->model()->index(row, 1).data().toString();
    QString adresse = ui->gpcTableView->model()->index(row, 2).data().toString();
    int niveau = ui->gpcTableView->model()->index(row, 3).data().toInt();
    QString dateStr = ui->gpcTableView->model()->index(row, 4).data().toString();
    QString typeZone = ui->gpcTableView->model()->index(row, 5).data().toString();

    ui->GPC_FormCmbZone->setCurrentText(zone);
    ui->GPC_FormTxtAddress->setText(adresse);
    ui->GPC_FormLevel->setValue(niveau);

    QDate d = QDate::fromString(dateStr.left(10), "yyyy-MM-dd");
    if (d.isValid())
        ui->GPC_FormDate->setDate(d);

    ui->GPC_FormCmbPriority->setCurrentText(typeZone);

    QMessageBox::information(this, "Point sélectionné",
                             "Point sélectionné. Tu peux modifier les champs ou cliquer sur Mesurer pour mettre à jour le niveau automatiquement." );
    //new
    resetPredictionFields();
}

void WasteRecycling::on_GPC_BtnModifier_clicked()
{
    if (gpcSelectedId == -1)
    {
        QMessageBox::warning(this, "GPC", "Sélectionne une ligne à modifier.");
        return;
    }

    QString zone = ui->GPC_FormCmbZone->currentText();
    QString adresse = ui->GPC_FormTxtAddress->text();
    int niveau = ui->GPC_FormLevel->value();
    QString date = ui->GPC_FormDate->date().toString("yyyy-MM-dd");
    QString typeZone = ui->GPC_FormCmbPriority->currentText();

    PointCollecte pc(0, zone, adresse, niveau, date, typeZone);

    if (pc.modifier(gpcSelectedId))
    {
        ui->gpcTableView->setModel(PC.afficher());
        ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));

        QMessageBox::information(this, "GPC", "Modification réussie !");

        // Infiltration Camion
        adresse += ", Délégation Raoued, Gouvernorat Ariana, Tunisie";
        CamTemp.camConvModifCoord(gpcSelectedId,adresse);
        // 😈😈

        gpcSelectedId = -1;

        ui->GPC_FormCmbZone->setCurrentIndex(0);
        ui->GPC_FormTxtAddress->clear();
        ui->GPC_FormLevel->setValue(0);
        ui->GPC_FormDate->setDate(QDate::currentDate());
        ui->GPC_FormCmbPriority->setCurrentIndex(0);
        //new
        resetPredictionFields();
        gpcSelectedId = -1;
        ui->GPC_BtnPredict->setEnabled(false);
    }
    else
    {
        QMessageBox::critical(this, "GPC", "Erreur modification !");
    }
}

// Met à jour le tableau en temps réel selon le texte de recherche
void WasteRecycling::on_GPC_TxtSearch_textChanged(const QString &text)
{
    ui->gpcTableView->setModel(PC.rechercher(text));
}

// Applique le tri métier selon la priorité choisie
void WasteRecycling::on_GPC_CmbFilter_currentTextChanged(const QString &text)
{
    ui->gpcTableView->setModel(PC.trierParPriorite(text));
    ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));
}

// Exporte le contenu actuel du tableau des points de collecte en PDF
void WasteRecycling::on_GPC_BtnExport_clicked()
{
    QAbstractItemModel *model = ui->gpcTableView->model();

    if (!model || model->rowCount() == 0)
    {
        QMessageBox::warning(this, "Export PDF", "Aucune donnée à exporter.");
        return;
    }

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                          + "/Points_de_collecte_"
                          + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")
                          + ".pdf";

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Enregistrer le PDF",
        defaultPath,
        "Fichiers PDF (*.pdf)"
        );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
        fileName += ".pdf";

    QPdfWriter pdf(fileName);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Landscape);
    pdf.setResolution(120);

    QPainter painter(&pdf);
    if (!painter.isActive())
    {
        QMessageBox::critical(this, "Export PDF", "Impossible de créer le fichier PDF.");
        return;
    }

    int pageWidth = pdf.width();
    int pageHeight = pdf.height();

    int margin = 40;
    int y = margin;

    QFont titleFont("Arial", 16, QFont::Bold);
    QFont subTitleFont("Arial", 10);
    QFont headerFont("Arial", 10, QFont::Bold);
    QFont cellFont("Arial", 9);

    // Titre
    painter.setFont(titleFont);
    painter.drawText(margin, y, "Liste des points de collecte");
    y += 35;

    // Date export
    painter.setFont(subTitleFont);
    painter.drawText(margin, y, "Date d'export : " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
    y += 30;

    int columnCount = model->columnCount();
    int rowCount = model->rowCount();

    if (columnCount <= 0)
    {
        painter.end();
        QMessageBox::warning(this, "Export PDF", "Le tableau ne contient aucune colonne.");
        return;
    }

    int tableWidth = pageWidth - 2 * margin;
    int rowHeight = 28;
    int headerHeight = 30;
    int colWidth = tableWidth / columnCount;

    auto drawHeader = [&]()
    {
        painter.setFont(headerFont);

        for (int col = 0; col < columnCount; col++)
        {
            QRect rect(margin + col * colWidth, y, colWidth, headerHeight);
            painter.fillRect(rect, QColor(47, 111, 104));
            painter.setPen(Qt::white);
            painter.drawRect(rect);
            painter.drawText(rect, Qt::AlignCenter, model->headerData(col, Qt::Horizontal).toString());
        }

        y += headerHeight;
    };

    drawHeader();

    painter.setFont(cellFont);

    for (int row = 0; row < rowCount; row++)
    {
        // Saut de page si nécessaire
        if (y + rowHeight > pageHeight - margin)
        {
            pdf.newPage();
            y = margin;

            painter.setFont(titleFont);
            painter.drawText(margin, y, "Liste des points de collecte");
            y += 35;

            painter.setFont(subTitleFont);
            painter.drawText(margin, y, "Suite - page suivante");
            y += 30;

            drawHeader();
            painter.setFont(cellFont);
        }

        for (int col = 0; col < columnCount; col++)
        {
            QRect rect(margin + col * colWidth, y, colWidth, rowHeight);

            if (row % 2 == 0)
                painter.fillRect(rect, QColor(245, 245, 245));
            else
                painter.fillRect(rect, QColor(255, 255, 255));

            painter.setPen(Qt::black);
            painter.drawRect(rect);

            QString text = model->index(row, col).data().toString();
            painter.drawText(rect.adjusted(4, 0, -4, 0), Qt::AlignCenter, text);
        }

        y += rowHeight;
    }

    painter.end();

    QMessageBox::information(this, "Export PDF", "PDF exporté avec succès !\n\nEmplacement :\n" + fileName);
}

//Nettoyer un layout avant de recréer le graphique
void WasteRecycling::nettoyerLayout(QWidget *widget)
{
    if (!widget)
        return;

    QLayout *layout = widget->layout();
    if (!layout)
        return;

    while (QLayoutItem *item = layout->takeAt(0))
    {
        if (item->widget())
        {
            item->widget()->hide();
            delete item->widget();
        }
        delete item;
    }
}

//Charger les KPI
void WasteRecycling::chargerStatistiquesGPC()
{
    int total = PC.totalPoints();
    int urgents = PC.totalUrgents();
    double moyenne = PC.remplissageMoyen();
    int semaine = PC.collectesCetteSemaine();

    ui->GPC_LblTotalValue->setText(QString::number(total));
    ui->GPC_LblUrgentValue->setText(QString::number(urgents));
    ui->GPC_LblUrgentValue_2->setText(QString::number((int)moyenne) + " %");
    ui->GPC_LblWeekValue->setText(QString::number(semaine));

    creerChartBarZone();
    creerChartPiePriorite();
}

//Créer le bar chart par zone
void WasteRecycling::creerChartBarZone()

{
    nettoyerLayout(ui->GPC_ChartBar);

    QMap<QString, double> data = PC.moyenneParZone();

    QBarSet *set0 = new QBarSet("Remplissage");
    QStringList zones;

    for (auto it = data.begin(); it != data.end(); ++it)
    {
        zones << it.key();
        *set0 << it.value();
    }

    if (zones.isEmpty())
    {
        zones << "Aucune zone";
        *set0 << 0;
    }

    set0->setColor(QColor(54, 124, 138));

    QBarSeries *series = new QBarSeries();
    series->append(set0);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setTitle("Remplissage par Zone");
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);
    chart->setBackgroundBrush(Qt::transparent);
    chart->setPlotAreaBackgroundBrush(Qt::transparent);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(zones);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setLabelFormat("%.0f");
    axisY->setGridLineColor(QColor(220, 220, 220));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: transparent;");
    view->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->GPC_ChartBar->layout());
    if (!layout)
    {
        layout = new QVBoxLayout(ui->GPC_ChartBar);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
    }

    layout->addWidget(view);
}
//Créer le donut chart priorité

void WasteRecycling::creerChartPiePriorite()
{
    nettoyerLayout(ui->GPC_ChartPie);

    QMap<QString, int> data = PC.repartitionParPriorite();

    QPieSeries *series = new QPieSeries();
    series->append("Urgente", data.value("Urgente", 0));
    series->append("Normale", data.value("Normale", 0));
    series->append("Basse", data.value("Basse", 0));
    series->setHoleSize(0.45);

    QList<QPieSlice*> slices = series->slices();
    if (slices.size() >= 3)
    {
        slices[0]->setColor(QColor(173, 216, 230)); // clair
        slices[1]->setColor(QColor(79, 162, 213));  // moyen
        slices[2]->setColor(QColor(36, 112, 156));  // foncé
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->setAlignment(Qt::AlignLeft);
    chart->setTitle("Répartition par Priorité");
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);
    chart->setBackgroundBrush(Qt::transparent);
    chart->setPlotAreaBackgroundBrush(Qt::transparent);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: transparent;");
    view->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->GPC_ChartPie->layout());
    if (!layout)
    {
        layout = new QVBoxLayout(ui->GPC_ChartPie);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
    }

    layout->addWidget(view);
}

//Animation d’ouverture de la page stats

void WasteRecycling::animerPageStatistiqueGPC()
{
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(ui->GPC_PageStats);
    ui->GPC_PageStats->setGraphicsEffect(opacityEffect);

    QPropertyAnimation *fadeAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnim->setDuration(700);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::InOutQuad);

    QPoint barEnd = ui->GPC_ChartBar->pos();
    QPoint pieEnd = ui->GPC_ChartPie->pos();

    QPropertyAnimation *barAnim = new QPropertyAnimation(ui->GPC_ChartBar, "pos");
    barAnim->setDuration(700);
    barAnim->setStartValue(barEnd + QPoint(-40, 0));
    barAnim->setEndValue(barEnd);
    barAnim->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation *pieAnim = new QPropertyAnimation(ui->GPC_ChartPie, "pos");
    pieAnim->setDuration(700);
    pieAnim->setStartValue(pieEnd + QPoint(40, 0));
    pieAnim->setEndValue(pieEnd);
    pieAnim->setEasingCurve(QEasingCurve::OutCubic);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(fadeAnim);
    group->addAnimation(barAnim);
    group->addAnimation(pieAnim);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

//****************************************GPC
//************************gestion client
void WasteRecycling::chargerClients()
{
    // Supprimer l'ancien modèle proprement
    QAbstractItemModel *old = ui->cllistec->model();
    ui->cllistec->setModel(nullptr);
    delete old;
    m_model = nullptr;

    m_model = ClientTemp.afficherClient();

    if (m_model->lastError().isValid()) {
        QMessageBox::critical(this, "Erreur BDD",
                              "Impossible de charger les clients :\n" + m_model->lastError().text());
        return;
    }

    ui->cllistec->setModel(m_model);
    ui->cllistec->resizeColumnsToContents();
}

// ─── Retourne l'id_client de la ligne sélectionnée ───────────────────────────
int WasteRecycling::selectedId() const
{
    QModelIndex idx = ui->cllistec->currentIndex();
    if (!idx.isValid()) return -1;
    // colonne 0 = id_client dans la requête SELECT
    return ui->cllistec->model()->data(
                                    ui->cllistec->model()->index(idx.row(), 0)).toInt();
}

// ─── Helpers ─────────────────────────────────────────────────────────────────
bool  WasteRecycling::isValidEmail(const QString &email)
{
    return RE_EMAIL.match(email).hasMatch();
}

// ─── CRUD — Ajouter ──────────────────────────────────────────────────────────
void  WasteRecycling::on_clbtnajt_clicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un client");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(
        "QDialog{background:white;} QLabel{color:black;font-size:13px;}"
        "QLineEdit{color:black;background:white;border:1px solid #aaa;"
        "   border-radius:4px;padding:4px 6px;font-size:13px;}"
        "QLineEdit:focus{border:1.5px solid #185FA5;}"
        "QComboBox{color:black;background:white;border:1px solid #aaa;"
        "   border-radius:4px;padding:4px 6px;font-size:13px;}"
        "QComboBox QAbstractItemView{background:white;color:black;}"
        "QPushButton{font-size:13px;padding:6px 18px;border-radius:4px;"
        "   border:1px solid #aaa;background:white;color:black;}"
        "QPushButton#btnOk{background:#185FA5;color:white;border-color:#185FA5;}"
        "QPushButton#btnOk:hover{background:#0C447C;}"
        "QPushButton#btnAnn:hover{background:#f0f0f0;}");

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setContentsMargins(20, 20, 20, 10);
    form->setVerticalSpacing(10);

    QLineEdit *fNom   = new QLineEdit();
    QComboBox *fType  = new QComboBox();
    QLineEdit *fTel   = new QLineEdit();
    QLineEdit *fAdr   = new QLineEdit();
    QLineEdit *fEmail = new QLineEdit();
    QLineEdit *fTrans = new QLineEdit("0");

    fNom->setValidator(new QRegularExpressionValidator(RE_NOM, &dialog));
    fTel->setValidator(new QRegularExpressionValidator(RE_TEL, &dialog));
    fTrans->setValidator(new QIntValidator(0, 1'000'000, &dialog));
    fType->addItems({"municipalite", "entreprise", "individu"});

    form->addRow("Nom :",             fNom);
    form->addRow("Type :",            fType);
    form->addRow("Téléphone :",       fTel);
    form->addRow("Adresse :",         fAdr);
    form->addRow("Email :",           fEmail);
    form->addRow("Nb transactions :", fTrans);

    QPushButton *btnOk  = new QPushButton("Ajouter");  btnOk->setObjectName("btnOk");
    QPushButton *btnAnn = new QPushButton("Annuler");  btnAnn->setObjectName("btnAnn");

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(); btnRow->addWidget(btnAnn); btnRow->addWidget(btnOk);
    btnRow->setContentsMargins(20, 0, 20, 16);

    QVBoxLayout *mainL = new QVBoxLayout(&dialog);
    mainL->setSpacing(8); mainL->addLayout(form); mainL->addLayout(btnRow);

    connect(btnOk, &QPushButton::clicked, this, [&]() {
        if (fNom->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Ajouter", "Le nom est obligatoire.");
            fNom->setFocus();
            return;
        }
        if (!fEmail->text().isEmpty() && !isValidEmail(fEmail->text())) {
            QMessageBox::warning(&dialog, "Ajouter", "Adresse email invalide.");
            fEmail->setFocus();
            return;
        }
        if (!fTel->text().isEmpty() && !fTel->hasAcceptableInput()) {
            QMessageBox::warning(&dialog, "Ajouter", "Téléphone invalide (8-15 chiffres).");
            fTel->setFocus();
            return;
        }
        dialog.accept();
    });
    connect(btnAnn, &QPushButton::clicked, &dialog, &QDialog::reject); // ← reste pareil


    if (dialog.exec() != QDialog::Accepted) return;

    static const QStringList types = {"municipalite", "entreprise", "individu"};
    client nouveau(
        0,
        fNom->text(),
        types[fType->currentIndex()],
        fAdr->text(),
        0,
        fEmail->text(),
        fTel->text(),
        fTrans->text().toInt()
        );

    if (nouveau.ajouterClient()) {
        A.envoyerSignal("ADD");
        QMessageBox::information(this, "Ajouter", "Client ajouté avec succès.");
        chargerClients(); // rafraîchir la table
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout en base de données.");
    }
}

// ─── CRUD — Modifier ─────────────────────────────────────────────────────────
void  WasteRecycling::on_clbtnmodif_clicked()
{
    int id = selectedId();
    if (id < 0) {
        QMessageBox::warning(this, "Modifier", "Veuillez sélectionner un client."); return;
    }

    // Lire les données actuelles depuis le modèle
    QModelIndex cur = ui->cllistec->currentIndex();
    int row = cur.row();
    auto val = [&](int col) {
        return ui->cllistec->model()->data(
                                        ui->cllistec->model()->index(row, col)).toString();
    };

    // ordre SELECT: id_client(0) nom(1) type(2) telephone(3) adresse(4) email(5) points(6) nb_transactions(7)
    QString curNom   = val(1);
    QString curType  = val(2);
    QString curTel   = val(3);
    QString curAdr   = val(4);
    QString curEmail = val(5);
    QString curPts   = val(6);
    QString curTrans = val(7);

    QDialog dialog(this);
    dialog.setWindowTitle(QString("Modifier client ID %1").arg(id));
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(
        "QDialog{background:white;} QLabel{color:black;font-size:13px;}"
        "QLineEdit{color:black;background:white;border:1px solid #aaa;"
        "   border-radius:4px;padding:4px 6px;font-size:13px;}"
        "QLineEdit:focus{border:1.5px solid #185FA5;}"
        "QComboBox{color:black;background:white;border:1px solid #aaa;"
        "   border-radius:4px;padding:4px 6px;font-size:13px;}"
        "QComboBox QAbstractItemView{background:white;color:black;}"
        "QPushButton{font-size:13px;padding:6px 18px;border-radius:4px;"
        "   border:1px solid #aaa;background:white;color:black;}"
        "QPushButton#btnOk{background:#185FA5;color:white;border-color:#185FA5;}"
        "QPushButton#btnOk:hover{background:#0C447C;}"
        "QPushButton#btnAnn:hover{background:#f0f0f0;}");

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setContentsMargins(20, 20, 20, 10);
    form->setVerticalSpacing(10);

    QLineEdit *fNom   = new QLineEdit(curNom);
    QComboBox *fType  = new QComboBox();
    QLineEdit *fTel   = new QLineEdit(curTel);
    QLineEdit *fAdr   = new QLineEdit(curAdr);
    QLineEdit *fEmail = new QLineEdit(curEmail);
    QLineEdit *fTrans = new QLineEdit(curTrans);


    fNom->setValidator(new QRegularExpressionValidator(RE_NOM, &dialog));
    fTel->setValidator(new QRegularExpressionValidator(RE_TEL, &dialog));
    fTrans->setValidator(new QIntValidator(0, 1'000'000, &dialog));

    fType->addItems({"municipalite", "entreprise", "individu"});
    fType->setCurrentText(curType);

    form->addRow("Nom :",             fNom);
    form->addRow("Type :",            fType);
    form->addRow("Téléphone :",       fTel);
    form->addRow("Adresse :",         fAdr);
    form->addRow("Email :",           fEmail);


    QPushButton *btnOk  = new QPushButton("Confirmer"); btnOk->setObjectName("btnOk");
    QPushButton *btnAnn = new QPushButton("Annuler");   btnAnn->setObjectName("btnAnn");

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(); btnRow->addWidget(btnAnn); btnRow->addWidget(btnOk);
    btnRow->setContentsMargins(20, 0, 20, 16);

    QVBoxLayout *mainL = new QVBoxLayout(&dialog);
    mainL->setSpacing(8); mainL->addLayout(form); mainL->addLayout(btnRow);

    connect(btnOk,  &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(btnAnn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    if (fNom->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Modifier", "Le nom ne peut pas être vide."); return;
    }
    if (!fEmail->text().isEmpty() && !isValidEmail(fEmail->text())) {
        QMessageBox::warning(this, "Modifier", "Adresse email invalide."); return;
    }
    if (!fTel->text().isEmpty() && !fTel->hasAcceptableInput()) {
        QMessageBox::warning(this, "Modifier", "Téléphone invalide (8-15 chiffres)."); return;
    }

    static const QStringList types = {"municipalite", "entreprise", "individu"};
    client modifie(
        id,
        fNom->text(),
        types[fType->currentIndex()],
        fAdr->text(),
        0,
        fEmail->text(),
        fTel->text(),
        fTrans->text().toInt()
        );

    if (modifie.modifierClient()) {
        A.envoyerSignal("MOD");
        QMessageBox::information(this, "Modifier", "Client modifié avec succès.");
        chargerClients();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification en base de données.");
    }
}

// ─── CRUD — Supprimer un client (clic droit sur ligne) ───────────────────────
void  WasteRecycling::on_clbtnsup_clicked()
{
    int id = selectedId();

    // Si rien n'est sélectionné → supprimer TOUS
    if (id < 0) {
        if (m_model && m_model->rowCount() == 0) {
            QMessageBox::information(this, "Info", "La liste est déjà vide."); return;
        }
        if (QMessageBox::question(this, "Suppression",
                                  "Aucun client sélectionné.\nSupprimer TOUS les clients ?") != QMessageBox::Yes)
            return;

        client modele;
        if (modele.supprimerTousClients()) {
            A.envoyerSignal("DEL");
            QMessageBox::information(this, "Suppression", "Tous les clients supprimés.");
            chargerClients();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression.");
        }
        return;
    }

    // Supprimer le client sélectionné
    if (QMessageBox::question(this, "Suppression",
                              QString("Supprimer le client ID %1 ?").arg(id)) != QMessageBox::Yes)
        return;

    client modele;
    if (modele.supprimerClient(id)) {
        A.envoyerSignal("DEL");
        QMessageBox::information(this, "Suppression", "Client supprimé avec succès.");
        chargerClients();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression.");
    }
}
// ─── double clic supprimer ────────────────────────────────────────
void WasteRecycling::on_cllistec_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    int id = ui->cllistec->model()->data(
                                      ui->cllistec->model()->index(index.row(), 0)).toInt();
    if (QMessageBox::question(this, "Suppression",
                              QString("Supprimer le client ID %1 ?").arg(id)) == QMessageBox::Yes) {
        client modele;
        if (modele.supprimerClient(id)) {
            A.envoyerSignal("DEL");
            QMessageBox::information(this, "Suppression", "Client supprimé avec succès.");
            chargerClients();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression.");
        }
    }
}
// ─── Recherche — filtre en temps réel ────────────────────────────────────────
void  WasteRecycling::on_clsearch_textChanged(const QString &text)
{
    if (!m_model) return;

    // On utilise un QSortFilterProxyModel pour filtrer sans toucher au modèle source
    QSortFilterProxyModel *proxy = qobject_cast<QSortFilterProxyModel*>(ui->cllistec->model());

    if (!proxy) {
        proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(m_model);
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxy->setFilterKeyColumn(-1); // chercher dans toutes les colonnes
        ui->cllistec->setModel(proxy);
    }

    proxy->setFilterFixedString(text);
}

// ─── Tri ─────────────────────────────────────────────────────────────────────
void  WasteRecycling::on_clbtntri_clicked()
{
    if (!m_model) return;

    QSortFilterProxyModel *proxy = qobject_cast<QSortFilterProxyModel*>(ui->cllistec->model());
    if (!proxy) {
        proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(m_model);
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxy->setFilterKeyColumn(-1);
        ui->cllistec->setModel(proxy);
    }

    int col = ui->clcombosort->currentIndex();
    proxy->sort(col, Qt::AscendingOrder);

    QMessageBox::information(this, "Tri", "Trié par : " + ui->clcombosort->currentText());
}

// ─── Export PDF ───────────────────────────────────────────────────────────────
void  WasteRecycling::on_clbtnpdf_clicked()
{
    if (!m_model || m_model->rowCount() == 0) {
        QMessageBox::information(this, "Export PDF", "Aucun client à exporter."); return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF",
                                                    "clients_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf", "PDF (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Landscape);

    // Récupérer le modèle affiché (proxy ou direct)
    QAbstractItemModel *displayed = ui->cllistec->model();
    int n = displayed->rowCount();

    static const QStringList TYPE_LABELS = {"municipalite", "entreprise", "individu"};
    static const QStringList TYPE_CSS    = {"badge-m", "badge-e", "badge-i"};

    QString html =
        "<html><head><meta charset='utf-8'><style>"
        "body{font-family:Arial,sans-serif;font-size:10pt;color:#111;}"
        "h2{color:#185FA5;margin:0 0 4px;font-size:16pt;}"
        ".meta{font-size:9pt;color:#666;margin-bottom:16px;}"
        "table{width:100%;border-collapse:collapse;margin-top:8px;}"
        "th{background:#185FA5;color:white;padding:6px 10px;text-align:left;font-size:9pt;}"
        "td{padding:5px 10px;border-bottom:1px solid #ddd;font-size:9pt;}"
        "tr:nth-child(even) td{background:#f0f5fb;}"
        ".badge-m{color:#0C447C;}.badge-e{color:#993C1D;}.badge-i{color:#3B6D11;}"
        "</style></head><body>"
        "<h2>Liste des Clients</h2>"
        "<p class='meta'>Date : <b>" + QDate::currentDate().toString("dd/MM/yyyy") +
        "</b> &nbsp;|&nbsp; " + QString::number(n) + " client(s)</p>"
                                                     "<table><thead><tr>"
                                                     "<th>ID</th><th>Nom</th><th>Type</th><th>Téléphone</th>"
                                                     "<th>Adresse</th><th>Email</th><th>Points</th><th>Nb transactions</th>"
                                                     "</tr></thead><tbody>";

    for (int i = 0; i < n; i++) {
        auto v = [&](int col) {
            return displayed->data(displayed->index(i, col)).toString().toHtmlEscaped();
        };
        html += "<tr>"
                "<td>" + v(0) + "</td>"
                         "<td><b>" + v(1) + "</b></td>"
                         "<td>" + v(2) + "</td>"
                         "<td>" + v(3) + "</td>"
                         "<td>" + v(4) + "</td>"
                         "<td>" + v(5) + "</td>"
                         "<td style='text-align:center'>" + v(6) + "</td>"
                         "<td style='text-align:center'>" + v(7) + "</td>"
                         "</tr>";
    }
    html += "</tbody></table></body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Export PDF", "PDF exporté !\n" + fileName);
}

// ─── Statistiques ─────────────────────────────────────────────────────────────
void  WasteRecycling::on_clbtnstat_clicked()
{
    if (!m_model || m_model->rowCount() == 0) {
        QMessageBox::information(this, "Statistiques", "Aucun client à afficher."); return;
    }

    int nb[3] = {0, 0, 0};
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString type = m_model->data(m_model->index(i, 2)).toString().toLower();
        if      (type == "municipalite") nb[0]++;
        else if (type == "entreprise")   nb[1]++;
        else                             nb[2]++;
    }

    static const QStringList labels = {"Municipalité", "Entreprise", "Individu"};
    QPieSeries *series = new QPieSeries();
    for (int i = 0; i < 3; i++)
        if (nb[i] > 0) series->append(QString("%1 (%2)").arg(labels[i]).arg(nb[i]), nb[i]);
    for (auto *s : series->slices()) s->setLabelVisible(true);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des clients");
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);

    QLayoutItem *child;
    while ((child = ui->verticalLayout_stat->takeAt(0))) { delete child->widget(); delete child; }
    ui->verticalLayout_stat->addWidget(view);
    ui->stackedWidgetclient->setCurrentIndex(PAGE_STATCLIENT);
}

void WasteRecycling::on_clretour1_clicked()    { ui->stackedWidgetclient->setCurrentIndex(PAGE_CRUDCLIENT);    }
void WasteRecycling::on_clretour2_clicked()    { ui->stackedWidgetclient->setCurrentIndex(PAGE_CRUDCLIENT);    }
void WasteRecycling::on_clretour3_clicked()    { ui->stackedWidgetclient->setCurrentIndex(PAGE_CRUDCLIENT);    }
void WasteRecycling::on_clbtnsysfide_clicked() { ui->stackedWidgetclient->setCurrentIndex(PAGE_SYSFIDECLIENT); }
void WasteRecycling::on_clbtnsysprio_clicked() { ui->stackedWidgetclient->setCurrentIndex(PAGE_SYSPRIOCLIENT); }

//************************gestion client

// GPC Retourne une couleur d’affichage selon le niveau de remplissage
QColor WasteRecycling::getFillColor(int niveau)
{
    if (niveau < 50)
        return QColor(76, 175, 80);    // vert

    if (niveau < 80)
        return QColor(255, 152, 0);   // orange

    return QColor(244, 67, 54);       // rouge
}

// Lance la prédiction de saturation pour le point sélectionné
// puis affiche les résultats dans l’interface
void WasteRecycling::on_GPC_BtnPredict_clicked()
{

    PointCollecte::PredictionResult res = PC.predireSaturation(gpcSelectedId);

    ui->GPC_LblPredictDays->setText(QString::number(res.joursRestants) + " jours");
    ui->GPC_LblPredictDate->setText(res.dateEstimee);
    ui->GPC_LblPredictRate->setText(QString::number(res.tauxParJour, 'f', 1) + " % / jour");

    if(res.fiable)
        ui->GPC_LblPredictRate->setStyleSheet("color: green;");
    else
        ui->GPC_LblPredictRate->setStyleSheet("color: orange;");
}

// Réinitialise les champs visuels de la prédiction
void WasteRecycling::resetPredictionFields()
{
    ui->GPC_LblPredictDays->setText("--");
    ui->GPC_LblPredictDate->setText("--");
    ui->GPC_LblPredictRate->setText("--");

    ui->GPC_LblPredictDays->setStyleSheet("");
    ui->GPC_LblPredictDate->setStyleSheet("");
    ui->GPC_LblPredictRate->setStyleSheet("");
}

// Charge et construit le graphique de classification d’urgence par zone
// à partir des résultats renvoyés par la classe métier
void WasteRecycling::chargerClassificationGPC()
{
    qDebug() << "chargerClassificationGPC appelée";

    if (!ui->GPC_ClassChartContainer)
    {
        qDebug() << "ERREUR : GPC_ClassChartContainer introuvable";
        return;
    }

    // Nettoyer ancien contenu
    QLayout *oldLayout = ui->GPC_ClassChartContainer->layout();
    if (oldLayout)
    {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr)
        {
            if (item->widget())
                delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    // Récupération depuis la classe métier
    QVector<PointCollecte::ZoneUrgence> data = PC.classerZonesParUrgence();

    if (data.isEmpty())
    {
        QLabel *msg = new QLabel("Aucune donnée disponible");
        msg->setAlignment(Qt::AlignCenter);
        msg->setStyleSheet("color: rgb(120,120,120); font-size: 18px; font-weight: bold;");

        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(msg);
        ui->GPC_ClassChartContainer->setLayout(layout);
        return;
    }

    QStringList zones;
    for (const PointCollecte::ZoneUrgence &z : data)
        zones << z.zone;

    QColor colorFaible(76, 175, 80);
    QColor colorMoyenne(241, 181, 0);
    QColor colorElevee(242, 140, 40);
    QColor colorCritique(220, 53, 69);
    QColor colorLine(31, 94, 78);
    QColor colorGrid(220, 225, 220);

    // Ligne principale
    QLineSeries *lineSeries = new QLineSeries();
    for (int i = 0; i < data.size(); ++i)
        lineSeries->append(i, data[i].urgenceValeur);

    QPen linePen(colorLine);
    linePen.setWidth(3);
    lineSeries->setPen(linePen);

    // Séries de points colorés
    QScatterSeries *seriesFaible = new QScatterSeries();
    seriesFaible->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    seriesFaible->setMarkerSize(16.0);
    seriesFaible->setColor(colorFaible);
    seriesFaible->setBorderColor(colorFaible);

    QScatterSeries *seriesMoyenne = new QScatterSeries();
    seriesMoyenne->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    seriesMoyenne->setMarkerSize(16.0);
    seriesMoyenne->setColor(colorMoyenne);
    seriesMoyenne->setBorderColor(colorMoyenne);

    QScatterSeries *seriesElevee = new QScatterSeries();
    seriesElevee->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    seriesElevee->setMarkerSize(16.0);
    seriesElevee->setColor(colorElevee);
    seriesElevee->setBorderColor(colorElevee);

    QScatterSeries *seriesCritique = new QScatterSeries();
    seriesCritique->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    seriesCritique->setMarkerSize(16.0);
    seriesCritique->setColor(colorCritique);
    seriesCritique->setBorderColor(colorCritique);

    for (int i = 0; i < data.size(); ++i)
    {
        switch (data[i].urgenceValeur)
        {
        case 1: seriesFaible->append(i, 1); break;
        case 2: seriesMoyenne->append(i, 2); break;
        case 3: seriesElevee->append(i, 3); break;
        case 4: seriesCritique->append(i, 4); break;
        }
    }

    // Chart
    QChart *chart = new QChart();
    chart->addSeries(lineSeries);
    chart->addSeries(seriesFaible);
    chart->addSeries(seriesMoyenne);
    chart->addSeries(seriesElevee);
    chart->addSeries(seriesCritique);

    chart->legend()->hide();
    chart->setTitle("");
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));

    // Axe X
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(zones);
    axisX->setGridLineVisible(true);
    axisX->setGridLinePen(QPen(colorGrid, 1, Qt::DashLine));
    axisX->setLabelsBrush(QBrush(QColor(40, 40, 40)));
    axisX->setLabelsFont(QFont("Arial", 10, QFont::Bold));

    chart->addAxis(axisX, Qt::AlignBottom);

    // Axe Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0.9, 4.1);
    axisY->setTickCount(4);
    axisY->setLabelsVisible(false);
    axisY->setGridLineVisible(true);
    axisY->setGridLinePen(QPen(colorGrid, 1, Qt::DashLine));

    chart->addAxis(axisY, Qt::AlignLeft);

    // Attacher axes
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    seriesFaible->attachAxis(axisX);
    seriesFaible->attachAxis(axisY);

    seriesMoyenne->attachAxis(axisX);
    seriesMoyenne->attachAxis(axisY);

    seriesElevee->attachAxis(axisX);
    seriesElevee->attachAxis(axisY);

    seriesCritique->attachAxis(axisX);
    seriesCritique->attachAxis(axisY);

    // Vue
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: transparent; border: none;");
    chartView->setMinimumHeight(260);

    // Panneau gauche
    QWidget *leftPanel = new QWidget();
    leftPanel->setFixedWidth(95);

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 30, 0, 30);
    leftLayout->setSpacing(22);

    QLabel *lbl4 = new QLabel("4\nCritique");
    lbl4->setAlignment(Qt::AlignCenter);
    lbl4->setStyleSheet("color: rgb(220,53,69); font-weight: bold; font-size: 18px; background: transparent; border: none;");

    QLabel *lbl3 = new QLabel("3\nÉlevée");
    lbl3->setAlignment(Qt::AlignCenter);
    lbl3->setStyleSheet("color: rgb(242,140,40); font-weight: bold; font-size: 18px; background: transparent; border: none;");

    QLabel *lbl2 = new QLabel("2\nMoyenne");
    lbl2->setAlignment(Qt::AlignCenter);
    lbl2->setStyleSheet("color: rgb(241,181,0); font-weight: bold; font-size: 18px; background: transparent; border: none;");

    QLabel *lbl1 = new QLabel("1\nFaible");
    lbl1->setAlignment(Qt::AlignCenter);
    lbl1->setStyleSheet("color: rgb(76,175,80); font-weight: bold; font-size: 18px; background: transparent; border: none;");

    leftLayout->addWidget(lbl4);
    leftLayout->addWidget(lbl3);
    leftLayout->addWidget(lbl2);
    leftLayout->addWidget(lbl1);

    // Centre
    QWidget *centerPanel = new QWidget();
    QVBoxLayout *centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(6);

    QLabel *xTitle = new QLabel("Zones");
    xTitle->setAlignment(Qt::AlignCenter);
    xTitle->setStyleSheet(
        "font-weight: bold;"
        "color: rgb(40,40,40);"
        "font-size: 15px;"
        "background: transparent;"
        "border: none;"
        );
    xTitle->setFixedHeight(24);

    centerLayout->addWidget(chartView, 1);
    centerLayout->addWidget(xTitle);

    // Légende droite
    QWidget *rightPanel = new QWidget();
    rightPanel->setFixedWidth(155);
    rightPanel->setStyleSheet(
        "background-color: rgb(245,248,243);"
        "border: 1px solid rgb(210,220,210);"
        "border-radius: 12px;"
        );

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(15, 15, 15, 15);
    rightLayout->setSpacing(15);

    QLabel *legendTitle = new QLabel("Légende");
    legendTitle->setAlignment(Qt::AlignCenter);
    legendTitle->setStyleSheet("font-weight: bold; font-size: 18px; color: rgb(31,94,78); border:none;");

    auto createLegendItem = [&](const QString &text, const QColor &color) -> QWidget*
    {
        QWidget *item = new QWidget();
        item->setStyleSheet("border:none;");
        QHBoxLayout *layout = new QHBoxLayout(item);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);

        QLabel *dot = new QLabel("●");
        dot->setStyleSheet(QString("color:%1; font-size:22px; border:none;").arg(color.name()));

        QLabel *txt = new QLabel(text);
        txt->setStyleSheet("font-size:15px; color: rgb(40,40,40); border:none;");

        layout->addWidget(dot);
        layout->addWidget(txt);
        layout->addStretch();

        return item;
    };

    rightLayout->addWidget(legendTitle);
    rightLayout->addWidget(createLegendItem("4 = Critique", colorCritique));
    rightLayout->addWidget(createLegendItem("3 = Élevée", colorElevee));
    rightLayout->addWidget(createLegendItem("2 = Moyenne", colorMoyenne));
    rightLayout->addWidget(createLegendItem("1 = Faible", colorFaible));
    rightLayout->addStretch();

    // Layout final
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(18);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(centerPanel, 1);
    mainLayout->addWidget(rightPanel);

    ui->GPC_ClassChartContainer->setLayout(mainLayout);
}

// Charge le tableau détaillé de classification d’urgence
// avec interprétation et action recommandée
void WasteRecycling::chargerTableClassificationGPC()
{
    QVector<PointCollecte::ZoneUrgence> zones = PC.classerZonesParUrgence();

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal, "Zone");
    model->setHeaderData(1, Qt::Horizontal, "Niveau d'urgence");
    model->setHeaderData(2, Qt::Horizontal, "Interprétation");
    model->setHeaderData(3, Qt::Horizontal, "Action recommandée");

    auto createItem = [](const QString &text) -> QStandardItem*
    {
        QStandardItem *item = new QStandardItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        item->setEditable(false);
        return item;
    };

    for (const PointCollecte::ZoneUrgence &z : zones)
    {
        QList<QStandardItem*> row;
        row << createItem(z.zone)
            << createItem(z.urgenceTexte)
            << createItem(z.interpretation)
            << createItem(z.action);

        row[1]->setForeground(QBrush(z.couleur));
        row[1]->setFont(QFont("Arial", 10, QFont::Bold));

        model->appendRow(row);
    }

    ui->GPC_ClassTableView->setModel(model);
    ui->GPC_ClassTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->GPC_ClassTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->GPC_ClassTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->GPC_ClassTableView->verticalHeader()->setVisible(false);
    ui->GPC_ClassTableView->horizontalHeader()->setStretchLastSection(true);
    ui->GPC_ClassTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->GPC_ClassTableView->setShowGrid(true);

    ui->GPC_ClassTableView->setStyleSheet(
        "QTableView {"
        "   background-color: rgb(250,250,250);"
        "   border: 1px solid rgb(210,220,210);"
        "   border-radius: 12px;"
        "   gridline-color: rgb(220,228,220);"
        "   font-size: 15px;"
        "}"
        "QHeaderView::section {"
        "   background-color: rgb(31,94,78);"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 15px;"
        "   border: none;"
        "   padding: 8px;"
        "}"
        "QTableView::item {"
        "   padding: 8px;"
        "}"
        );
}


void WasteRecycling::on_GPC_BtnMeasure_clicked(){

    if (gpcSelectedId == -1)
    {
        QMessageBox::warning(this, "Arduino",
                             "Sélectionne d'abord un point de collecte dans le tableau.");
        return;
    }

    //Verifier que Arduino est connecté
    if (!A.getserial()->isOpen())
    {
        QMessageBox::warning(this, "Arduino", "Arduino n'est pas connecté.");
        return;
    }

    // On mémorise l'ID du point à modifier
    gpcIdMesure = gpcSelectedId;
    gpcMesureEnCours = true;

    //demander à Arduino de mesurer
    A.demanderMesure();

    QMessageBox::information(this, "Mesure", "Mesure demandée à Arduino.");
}

void WasteRecycling::update_label(QByteArray data){

    //convertir QByteArray vers QString
    QString valeur =QString::fromUtf8(data).trimmed();
    qDebug() <<"Donne recue de arduino : " << valeur;

    if(valeur.isEmpty()){
        return;
    }

    bool ok;
    int niveau =valeur.toInt(&ok);

    if (!ok){
        qDebug() <<"valeur arduinoi invalide :" <<valeur;
        return;}
    if(niveau<0){niveau=0;}
    if(niveau>100){niveau=100;}
    ui->GPC_FormLevel->setValue(niveau);
    qDebug()<<"Niveau mis dans le formulaire :" << niveau;

    // Vérifier qu'une mesure a bien été demandée par le bouton Mesurer
    if (!gpcMesureEnCours || gpcIdMesure == -1)
    {
        qDebug() << "Aucune mesure en cours.";
        return;
    }
    // Mettre à jour directement la base
    if (PC.modifierNiveauArduino(gpcIdMesure, niveau))
    {
        qDebug() << "Niveau Arduino enregistre dans la base pour ID_PC =" << gpcIdMesure;

        // Rafraîchir le tableau
        ui->gpcTableView->setModel(PC.afficher());
        ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));

        QMessageBox::information(this, "Arduino",
                                 "Niveau mesuré et enregistré directement dans la base.");

        // Réinitialiser l'état
        gpcMesureEnCours = false;
        gpcIdMesure = -1;
    }else
    {
        QMessageBox::critical(this, "Arduino",
                              "Erreur lors de l'enregistrement du niveau mesuré.");
    }
}
// Messagerie : initialise la liste + génère un message personnalisé par client
void WasteRecycling::setupMessagerieIA() {
    ui->cl_btnGenerer->setEnabled(false); // désactivé jusqu'à sélection d'un client

    // Charge la liste des clients (id+nom) depuis client.cpp
    QList<QPair<int,QString>> liste = client::getListeNomsClients();
    for (const QPair<int,QString> &entry : liste) {
        QListWidgetItem *item = new QListWidgetItem(entry.second); // affiche le nom
        item->setData(Qt::UserRole, entry.first);                  // stocke l'id en caché
        ui->cl_listeMsg->addItem(item);
    }

    // Clic sur un client → génère le message personnalisé
    connect(ui->cl_listeMsg, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int clientId = item->data(Qt::UserRole).toInt();
        QString nom  = item->text();

        // Récupère email + points + type depuis client.cpp (pas de SQL ici)
        client::ClientMailInfo info = client::getClientMailInfo(clientId);
        if (info.email.isEmpty() && info.points == 0 && info.type.isEmpty()) return;

        QString email = info.email;
        int     pts   = info.points;
        QString type  = info.type;

        // Calcul du niveau de fidélité
        QString niveau, reduction;
        if      (pts < 100) { niveau = "Bronze"; reduction = "5%";  }
        else if (pts < 300) { niveau = "Silver"; reduction = "10%"; }
        else if (pts < 600) { niveau = "Gold";   reduction = "15%"; }
        else                { niveau = "VIP";    reduction = "20%"; }

        // Calcul de l'état du stock des conteneurs
        int qteActuelle = 0, capMax = 0;
        client::getStockConteneurs(qteActuelle, capMax);
        int pourcentage = (capMax > 0) ? (qteActuelle * 100 / capMax) : 0;
        QString etatStock;
        if      (pourcentage == 0)  etatStock = "malheureusement en rupture de stock pour le moment";
        else if (pourcentage < 25)  etatStock = "actuellement en stock faible, une collecte sera bientôt disponible";
        else if (pourcentage < 65)  etatStock = "disponible normalement pour planifier votre collecte";
        else                        etatStock = "bien rempli et disponible pour votre collecte";

        // Construction du message personnalisé
        QString message =
            "Bonjour " + nom + ",\n\n"
                               "Nous avons le plaisir de vous informer que, en tant que client "
            + type + " fidèle de WasteRecycling, vous bénéficiez actuellement "
                     "d'une réduction exclusive de " + reduction + " sur nos services.\n\n"
                          "Cette réduction est attribuée grâce à votre niveau de fidélité : "
            + niveau + " (" + QString::number(pts) + " points accumulés).\n\n"
                                                     "Concernant notre service de collecte, notre stock est " + etatStock + ".\n\n"
                          "Nous vous remercions de votre confiance et restons à votre disposition "
                          "pour toute question ou planification de collecte.\n\n"
                          "Cordialement,\nL'équipe WasteRecycling";

        ui->cl_messageIA->setText(message);
        ui->cl_lblClientIA->setText("Client : " + nom + " | " + email);
        ui->cl_btnGenerer->setEnabled(!email.isEmpty()); // active si email existe
        ui->cl_btnGenerer->setProperty("email", email); // stocke email dans propriété bouton
        ui->cl_btnGenerer->setProperty("nom", nom);
    });

    // Bouton Envoyer → ouvre le client mail avec l'URL mailto
    connect(ui->cl_btnGenerer, &QPushButton::clicked, this, [this]() {
        QString email   = ui->cl_btnGenerer->property("email").toString();
        QString message = ui->cl_messageIA->toPlainText();
        if (email.isEmpty()) { QMessageBox::warning(this, "Email", "Ce client n'a pas d'adresse email."); return; }
        QString sujet  = "WasteRecycling - Votre réduction fidélité";
        QString mailto = "mailto:" + email +
                         "?subject=" + QUrl::toPercentEncoding(sujet) +
                         "&body="    + QUrl::toPercentEncoding(message);
        QDesktopServices::openUrl(QUrl(mailto)); // délègue l'envoi au client mail du système
    });
}

// Fidélisation : charge les niveaux et remplit le tableau avec couleurs
void WasteRecycling::chargerFidelite() {
    // Comptage par niveau depuis client.cpp
    int bronze = 0, silver = 0, gold = 0, vip = 0;
    client::getStatsFidelite(bronze, silver, gold, vip);
    ui->cl_lblBronzeCount->setText(QString::number(bronze));
    ui->cl_lblSilverCount->setText(QString::number(silver));
    ui->cl_lblGoldCount->setText(QString::number(gold));
    ui->cl_lblVipCount->setText(QString::number(vip));

    // Préparation du tableau
    ui->cl_tableFidelite->clear();
    ui->cl_tableFidelite->setColumnCount(4);
    ui->cl_tableFidelite->setHorizontalHeaderLabels({"Nom", "Points", "Niveau", "Récompense"});
    ui->cl_tableFidelite->horizontalHeader()->setStretchLastSection(true);
    ui->cl_tableFidelite->verticalHeader()->setVisible(false);
    ui->cl_tableFidelite->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->cl_tableFidelite->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Remplissage ligne par ligne depuis client.cpp (triés par points DESC)
    QList<client::ClientFideliteInfo> liste = client::getClientsFideliteData();
    int row = 0;
    for (const client::ClientFideliteInfo &data : liste) {
        QString nom = data.nom;
        int     pts = data.points;
        QString niveau, recompense;
        QColor  couleur;
        if      (pts < 100) { niveau = "Bronze"; recompense = "Reduction 5%";  couleur = QColor("#f5deb3"); }
        else if (pts < 300) { niveau = "Silver"; recompense = "Reduction 10%"; couleur = QColor("#e8e8e8"); }
        else if (pts < 600) { niveau = "Gold";   recompense = "Reduction 15%"; couleur = QColor("#fff4cc"); }
        else                { niveau = "VIP";    recompense = "Reduction 20%"; couleur = QColor("#e8d5ff"); }

        ui->cl_lblBronzeCount->setAlignment(Qt::AlignCenter);
        ui->cl_lblSilverCount->setAlignment(Qt::AlignCenter);
        ui->cl_lblGoldCount->setAlignment(Qt::AlignCenter);
        ui->cl_lblVipCount->setAlignment(Qt::AlignCenter);
        ui->cl_lblBronzeCount->setText("Bronze : " + QString::number(bronze));
        ui->cl_lblSilverCount->setText("Silver : " + QString::number(silver));
        ui->cl_lblGoldCount->setText("Gold : "     + QString::number(gold));
        ui->cl_lblVipCount->setText("VIP : "       + QString::number(vip));

        ui->cl_tableFidelite->insertRow(row);
        ui->cl_tableFidelite->setItem(row, 0, new QTableWidgetItem(nom));
        ui->cl_tableFidelite->setItem(row, 1, new QTableWidgetItem(QString::number(pts) + " pts"));
        ui->cl_tableFidelite->setItem(row, 2, new QTableWidgetItem(niveau));
        ui->cl_tableFidelite->setItem(row, 3, new QTableWidgetItem(recompense));
        for (int col = 0; col < 4; col++)
            ui->cl_tableFidelite->item(row, col)->setBackground(couleur); // couleur par niveau
        row++;
    }
}

// Génère un message local de fidélité (version simplifiée sans stock)
QString WasteRecycling::genererMessageLocal(QString nom, int points) {
    float reduction = (points < 100) ? 5 : (points < 300) ? 10 : (points < 600) ? 15 : 20;
    return "Objet : Offre de fidélité\n\nBonjour " + nom + ",\n\n"
                                                           "Vous bénéficiez d'une réduction de " + QString::number(reduction) + "% grâce à votre fidélité.\n\n"
                                          "Vos points : " + QString::number(points) + ".\n\n"
                                       "Cordialement,\nL'équipe WasteRecycling";
}
void WasteRecycling::envoyerAudioWhisper(const QString &wavPath)
{
    QFile *file = new QFile(wavPath, this);
    if (!file->open(QIODevice::ReadOnly)) return;

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"model\"");
    modelPart.setBody("whisper-large-v3-turbo");

    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"file\"; filename=\"audio.wav\"");
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/wav");
    audioPart.setBodyDevice(file);
    file->setParent(multiPart);

    multiPart->append(modelPart);
    multiPart->append(audioPart);

    QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/audio/transcriptions"));
    req.setRawHeader("Authorization", "Bearer " + m_groqKey.toUtf8());

    QNetworkReply *reply = m_network->post(req, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        QString texte = obj["text"].toString();
        reply->deleteLater();
        envoyerTexteAuLLM(texte);   // → étape suivante
    });
}
void WasteRecycling::envoyerTexteAuLLM(const QString &texte)
{
    QString prompt =
        "Extrait les informations client depuis ce texte et réponds UNIQUEMENT en JSON valide "
        "avec les clés : nom, telephone, email, adresse, type (municipalite/entreprise/individu), "
        "nb_transactions. Laisse vide si absent.\nTexte : " + texte;

    QJsonObject body;
    body["model"] = "claude-sonnet-4-20250514";  // ou "gemini-1.5-flash"
    body["max_tokens"] = 300;
    body["messages"] = QJsonArray{
        QJsonObject{{"role","user"},{"content", prompt}}
    };

    QNetworkRequest req(QUrl("https://api.anthropic.com/v1/messages"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("x-api-key", m_anthropicKey.toUtf8());
    req.setRawHeader("anthropic-version", "2023-06-01");

    QNetworkReply *reply = m_network->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        reply->deleteLater();
        // Extrait le texte JSON de la réponse Claude
        QString jsonStr = doc["content"][0]["text"].toString();
        // Nettoie les backticks si présents
        jsonStr.remove("```json").remove("```").trimmed();
        QJsonObject data = QJsonDocument::fromJson(jsonStr.toUtf8()).object();
        ouvrirDialoguePreRempli(data);
    });
}

void WasteRecycling::ouvrirDialoguePreRempli(const QJsonObject &data)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un client — note vocale");
    dialog.setMinimumWidth(400);

    QFormLayout *form = new QFormLayout();
    form->setContentsMargins(20, 20, 20, 10);
    form->setVerticalSpacing(10);

    QLineEdit *fNom   = new QLineEdit(data["nom"].toString());
    QLineEdit *fTel   = new QLineEdit(data["telephone"].toString());
    QLineEdit *fAdr   = new QLineEdit(data["adresse"].toString());
    QLineEdit *fEmail = new QLineEdit(data["email"].toString());
    QLineEdit *fTrans = new QLineEdit(data["nb_transactions"].toString());

    QWidget *typeRow = new QWidget();
    QHBoxLayout *typeLayout = new QHBoxLayout(typeRow);
    typeLayout->setContentsMargins(0, 0, 0, 0);
    typeLayout->setSpacing(6);
    QComboBox *fType = new QComboBox();
    fType->addItems({"municipalite", "entreprise", "individu"});
    QPushButton *micType = new QPushButton("🎤");
    micType->setObjectName("btnMic");
    micType->setFixedWidth(34);
    typeLayout->addWidget(fType);
    typeLayout->addWidget(micType);
    fType->setCurrentText(data["type"].toString());

    auto normaliserType = [](QString t) -> QString {
        t = t.toLower().trimmed();
        t.replace("é","e").replace("è","e").replace("ê","e")
            .replace("à","a").replace("â","a");
        t.remove(QRegularExpression("[^a-z]"));
        if (t.contains("muni"))                       return "municipalite";
        if (t.contains("entre")||t.contains("antre")) return "entreprise";
        if (t.contains("indiv")||t.contains("endiv")) return "individu";
        return "";
    };

    connect(micType, &QPushButton::clicked, this, [=]() {
        micType->setText("⏺"); micType->setEnabled(false);
        if (m_recorder) { m_recorder->stop(); delete m_recorder; m_recorder = nullptr; }
        if (m_session)  { delete m_session;   m_session  = nullptr; }
        m_session  = new QMediaCaptureSession(this);
        m_recorder = new QMediaRecorder(this);
        QAudioInput *audioInput = new QAudioInput(this);
        m_session->setAudioInput(audioInput);
        m_session->setRecorder(m_recorder);
        QMediaFormat format; format.setFileFormat(QMediaFormat::Wave);
        m_recorder->setMediaFormat(format);
        m_recorder->setAudioSampleRate(16000); m_recorder->setAudioChannelCount(1);
        m_recorder->setOutputLocation(QUrl::fromLocalFile(m_wavPath));
        m_recorder->record();
        QTimer::singleShot(3000, this, [=]() {
            if (m_recorder) m_recorder->stop();
            micType->setText("🎤"); micType->setEnabled(true);
            QFileInfo fi(m_wavPath);
            if (!fi.exists() || fi.size() < 1000) { QMessageBox::warning(this, "Micro", "Trop court."); return; }
            QFile *file = new QFile(m_wavPath, this);
            if (!file->open(QIODevice::ReadOnly)) return;
            QHttpMultiPart *mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            QHttpPart modelPart; modelPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"model\""); modelPart.setBody("whisper-large-v3-turbo");
            QHttpPart langPart;  langPart.setHeader(QNetworkRequest::ContentDispositionHeader,  "form-data; name=\"language\""); langPart.setBody("fr");
            QHttpPart audioPart; audioPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"file\"; filename=\"audio.wav\"");
            audioPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/wav"); audioPart.setBodyDevice(file); file->setParent(mp);
            mp->append(modelPart); mp->append(langPart); mp->append(audioPart);
            QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/audio/transcriptions"));
            req.setRawHeader("Authorization", "Bearer " + m_groqKey.toUtf8());
            QNetworkReply *reply = m_network->post(req, mp); mp->setParent(reply);
            connect(reply, &QNetworkReply::finished, this, [=]() {
                QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
                reply->deleteLater();
                QString typeNorm = normaliserType(obj["text"].toString().trimmed());
                if (!typeNorm.isEmpty()) fType->setCurrentText(typeNorm);
                else QMessageBox::warning(this, "Type non reconnu", "Dites : municipalité, entreprise ou individu");
            });
        });
    });

    form->addRow("Nom :",             fNom);
    form->addRow("Type :",            typeRow);
    form->addRow("Téléphone :",       fTel);
    form->addRow("Adresse :",         fAdr);
    form->addRow("Email :",           fEmail);
    form->addRow("Nb transactions :", fTrans);

    QPushButton *btnOk  = new QPushButton("Confirmer et ajouter");
    QPushButton *btnAnn = new QPushButton("Annuler");
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(btnAnn);
    btnRow->addWidget(btnOk);

    QVBoxLayout *mainL = new QVBoxLayout(&dialog);
    mainL->addLayout(form);
    mainL->addLayout(btnRow);

    connect(btnOk, &QPushButton::clicked, &dialog, [&]() {
        if (fNom->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Nom", "Le nom est obligatoire.");
            return;
        }
        dialog.accept();
    });
    connect(btnAnn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    static const QStringList types = {"municipalite", "entreprise", "individu"};
    client nouveau(0, fNom->text(), types[fType->currentIndex()],
                   fAdr->text(), 0, fEmail->text(), fTel->text(), fTrans->text().toInt());

    if (nouveau.ajouterClient()) {
        A.envoyerSignal("ADD");  // ✅ ISD1820 joue 1 fois
        QMessageBox::information(this, "Succès", "Client ajouté depuis note vocale.");
        chargerClients();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout : " +
                                                  QSqlDatabase::database().lastError().text());
    }
}
void WasteRecycling::attendreArduino(QLineEdit *champ, const QString &type)
{
    m_modeVocal  = VOCAL_CHAMP;
    m_champCible = champ;
    m_champType  = type;
    // Plus de popup — l'utilisateur appuie directement sur Arduino
}

void WasteRecycling::attendreArduinoSuppression()
{
    m_modeVocal  = VOCAL_SUPPRESSION;
    m_champCible = nullptr;
    m_champType  = "";
    // Plus de popup — l'utilisateur appuie directement sur Arduino
}
void WasteRecycling::traiterAudioArduino(const QString &wavPath)
{
    if (m_modeVocal == VOCAL_AUCUN) return;

    QFile *file = new QFile(wavPath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Erreur audio",
                              "Impossible de lire le fichier audio Arduino.");
        return;
    }

    QHttpMultiPart *mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"model\"");
    modelPart.setBody("whisper-large-v3-turbo");

    QHttpPart langPart;
    langPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       "form-data; name=\"language\"");
    langPart.setBody("fr");

    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"file\"; filename=\"audio.wav\"");
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/wav");
    audioPart.setBodyDevice(file);
    file->setParent(mp);

    mp->append(modelPart); mp->append(langPart); mp->append(audioPart);

    QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/audio/transcriptions"));
    req.setRawHeader("Authorization", "Bearer " + m_groqKey.toUtf8());
    QNetworkReply *reply = m_network->post(req, mp);
    mp->setParent(reply);

    ModeVocal modeSnapshot = m_modeVocal;  // capture avant reset
    m_modeVocal = VOCAL_AUCUN;

    connect(reply, &QNetworkReply::finished, this, [this, reply, modeSnapshot]() {
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        reply->deleteLater();
        QString texte = obj["text"].toString().trimmed();
        if (texte.isEmpty()) return;

        if (modeSnapshot == VOCAL_CHAMP)
            remplirChampDepuisTexte(texte);
        else if (modeSnapshot == VOCAL_SUPPRESSION)
            traiterCommandeSuppression(texte);
    });
}
void WasteRecycling::remplirChampDepuisTexte(const QString &texte)
{
    qDebug() << "[VOCAL] texte reçu :" << texte;
    qDebug() << "[VOCAL] type :" << m_champType;
    qDebug() << "[VOCAL] champ cible :" << (m_champCible ? "OK" : "NULL");

    if (!m_champCible) return;

    QString valeur = texte;
    valeur.remove(QRegularExpression("[.،,!?]+$"));

    qDebug() << "[VOCAL] valeur avant traitement :" << valeur;

    if (m_champType == "tel" || m_champType == "trans") {
        QMap<QString, QString> mots = {
            {"zéro","0"}, {"zero","0"},
            {"un","1"}, {"une","1"},
            {"deux","2"}, {"trois","3"},
            {"quatre","4"}, {"cinq","5"},
            {"six","6"}, {"sept","7"},
            {"huit","8"}, {"neuf","9"},
            {"dix","10"}, {"onze","11"},
            {"douze","12"}, {"treize","13"},
            {"quatorze","14"}, {"quinze","15"},
            {"seize","16"}, {"vingt","20"},
            {"trente","30"}, {"quarante","40"},
            {"cinquante","50"}, {"soixante","60"},
            {"cent","100"}
        };
        for (auto it = mots.begin(); it != mots.end(); ++it) {
            QRegularExpression re("\\b" + it.key() + "\\b",
                                  QRegularExpression::CaseInsensitiveOption);
            valeur.replace(re, it.value());
        }
        valeur.remove(QRegularExpression("[^0-9]"));
    }

    qDebug() << "[VOCAL] valeur finale :" << valeur;

    m_champCible->setText(valeur);
    m_champCible = nullptr;
    m_champType  = "";
}
void WasteRecycling::traiterCommandeSuppression(const QString &texte)
{
    QString t = texte.toLower();

    // ── "supprimer tous les clients" ──
    if (t.contains("tous")) {
        if (QMessageBox::question(this, "Suppression vocale",
                                  "Vous êtes sûr de vouloir supprimer TOUS les clients ?",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            client modele;
            if (modele.supprimerTousClients()) {
                A.envoyerSignal("DEL");
                QMessageBox::information(this, "Succès",
                                         "Tous les clients ont été supprimés.");
                chargerClients();
            } else {
                QMessageBox::critical(this, "Erreur", "Échec de la suppression.");
            }
        }
        return;
    }

    // ── "supprimer client ID X" ──
    QRegularExpression reId(R"((\d+))");
    QRegularExpressionMatch m = reId.match(t);
    if (m.hasMatch()) {
        int id = m.captured(1).toInt();
        if (QMessageBox::question(this, "Suppression vocale",
                                  QString("Vous êtes sûr de vouloir supprimer le client ID %1 ?").arg(id),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            client modele;
            if (modele.supprimerClient(id)) {
                A.envoyerSignal("DEL");  // ✅ ISD1820 joue 2 fois
                QMessageBox::information(this, "Succès",
                                         QString("Client ID %1 supprimé.").arg(id));
                chargerClients();
            } else {
                QMessageBox::critical(this, "Erreur",
                                      "Échec — ID introuvable ou erreur BDD.");
            }
        }
        return;
    }

    // ── Commande non reconnue ──
    QMessageBox::warning(this, "Commande non reconnue",
                         "Texte reçu : « " + texte + " »\n\n"
                                                     "Dites :\n"
                                                     "• « Supprimer client ID 5 »\n"
                                                     "• « Supprimer tous les clients »");
}
void WasteRecycling::on_clbtnvocSup_clicked()
{
    attendreArduinoSuppression();
}

