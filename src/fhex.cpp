#include "fhex.h"
#include <QDebug>
#include <sys/types.h>
#include <sys/stat.h>

Fhex::Fhex(QWidget *parent, QApplication *app, QString filepath)
    : QMainWindow(parent)
{
    this->app = app;
    this->setWindowTitle("Fhex");
    this->setWindowIcon(QIcon("/usr/share/icons/fhex.png"));

    /**
     * LOAD WINDOW SIZE FROM FILE SETTINGS
    **/
    string homePath = "";

#ifdef WINDOWS
    homePath = string(getenv("HOMEDRIVE")) + string(getenv("HOMEPATH")) + "\\AppData\\Local\\";
    settingsPath = homePath + "\\fhex\\";
#else
    homePath = string(getenv("HOME"));
    settingsPath = homePath + "/.fhex/";
#endif

    settingsFile = settingsPath + "settings.json";
    std::ifstream configFile(settingsFile);

    if (!configFile.good()) {
        cerr << "The file " << settingsFile << " is not accessible." << endl;
    } else {
        try {
            configFile >> this->jconfig;
        } catch (exception &e) {
            cerr << "Exception occurred while parsing json configuration file:" << endl << e.what() << endl;
        }
    }
    QRect screenSize = QDesktopWidget().availableGeometry(this);

    /*
     * DEFAULTS
     */

    // Window: Default is half of screen size & centered on screen
    int width = screenSize.width() * 0.5f;
    int height = screenSize.height() * 0.5f;
    int x_position = screenSize.width() / 2 - width / 2;
    int y_position = screenSize.height() / 2 - height / 2;
    bool maximized = false;

    // Pattern
    patternsEnabled = false;

    // Look for size & position values to overwrite
    if (!jconfig.is_null()) {
        if (!jconfig["WindowSettings"]["width"].is_null()) {
            width = jconfig["WindowSettings"]["width"].get<int>();
        }
        if (!jconfig["WindowSettings"]["height"].is_null()) {
            height = jconfig["WindowSettings"]["height"].get<int>();
        }
        if (!jconfig["WindowSettings"]["x_position"].is_null()) {
            x_position = jconfig["WindowSettings"]["x_position"].get<int>();
        }
        if (!jconfig["WindowSettings"]["y_position"].is_null()) {
            y_position = jconfig["WindowSettings"]["y_position"].get<int>();
        }
        if (!jconfig["WindowSettings"]["maximized"].is_null()) {
            maximized = jconfig["WindowSettings"]["maximized"].get<bool>();
        }
        if (!jconfig["Patterns"]["file"].is_null()) {
            patternsFile = jconfig["Patterns"]["file"].get<string>();
        }
        if (!jconfig["Patterns"]["enabled"].is_null()) {
            patternsEnabled = jconfig["Patterns"]["enabled"].get<bool>();
        }
    }

    /**
     * APPLY WINDOW SIZE
    **/

    this->move(x_position, y_position);
    this->resize(width, height);
    if (maximized) {
        this->setWindowState(Qt::WindowMaximized);
    }

    this->prev_vscrollbar_value = 0;
    this->prev_hscrollbar_value = 0;

    if (patternsEnabled) {
        // If Patterns enabled
        this->hexEditor = new HexEditor(patternsFile);
    } else {
        // No Patterns
        this->hexEditor = new HexEditor();
    }

    /** Menu Initialization **/
    QMenu *file;
    file = menuBar()->addMenu("&File");
    QAction *newFile = new QAction(QIcon::fromTheme("document-new"), "&New", this);
    QAction *openFile = new QAction(QIcon::fromTheme("folder-open"), "&Open", this);
    openFile->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
    QAction *chunkOpenFile = new QAction(QIcon::fromTheme("folder-open"), "&Chunk Loader", this);
    chunkOpenFile->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_H));
    QAction *diffFile = new QAction(QIcon::fromTheme("folder-open"), "&Diff..", this);
    QAction *saveFile = new QAction(QIcon::fromTheme("document-save"), "&Save", this);
    saveFile->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
    QAction *saveNewFile = new QAction(QIcon::fromTheme("document-save-as"), "&Save as ..", this);
    QAction *openNewWindow = new QAction(QIcon::fromTheme("window-new"), "&New Window", this);
    openNewWindow->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_N));
    // Add quit action (CTRL+Q) as not every Window Manager provides an X button
    QAction *quit = new QAction(QIcon::fromTheme("window-quit"), "&Quit", this);
    quit->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Q));

    file->addAction(newFile);
    file->addAction(openFile);
    file->addAction(chunkOpenFile);
    file->addAction(diffFile);
    file->addAction(saveFile);
    file->addAction(saveNewFile);
    file->addAction(openNewWindow);
    file->addSeparator();
    file->addAction(quit);

    QMenu *edit;
    edit = menuBar()->addMenu("&Edit");
    QAction *find = new QAction(QIcon::fromTheme("edit-find"), "&Find", this);
    find->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F));
    edit->addAction(find);
    QAction *convert = new QAction(QIcon::fromTheme("view-refresh"), "&Convert Bytes", this);
    convert->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_B));
    edit->addAction(convert);
    QAction *gotoOffset = new QAction(QIcon::fromTheme("arrow-right"), "&Goto Offset", this);
    gotoOffset->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_G));
    edit->addAction(gotoOffset);
    QAction *openTextViewer = new QAction(QIcon::fromTheme("text-field"), "&Open Text Viewer", this);
    openTextViewer->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_T));
    edit->addAction(openTextViewer);

    // Added
    QAction *openPatternsMenu = new QAction(QIcon::fromTheme("folder-open"), "&Open Pattern Definition", this);
    openPatternsMenu->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_D));
    edit->addAction(openPatternsMenu);

    // Toggle patterns display On/Off
    QAction *togglePatternsMenu = new QAction(QIcon::fromTheme("find"), "&Toggle Patterns", this);
    togglePatternsMenu->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_P));
    edit->addAction(togglePatternsMenu);

    QAction *menuOffsetList = new QAction(QIcon::fromTheme("find"), "&Show/Hide Offset List", this);
    menuOffsetList->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L));
    edit->addAction(menuOffsetList);
    QAction *menuBinaryChart = new QAction(QIcon::fromTheme("image"), "Show/Hide &Binary Chart", this);
    menuBinaryChart->setShortcut(QKeySequence(Qt::Key_F1));
    edit->addAction(menuBinaryChart);

    connect(menuBinaryChart, &QAction::triggered, this, &Fhex::on_menu_binchart_click);
    connect(newFile, &QAction::triggered, this, &Fhex::on_menu_new_file_click);
    connect(menuOffsetList, &QAction::triggered, this, &Fhex::on_menu_offset_list_click);

    // Toggle patterns display On/Off
    connect(togglePatternsMenu, &QAction::triggered, this, &Fhex::on_menu_toggle_patterns_click);

    // Added
    connect(openPatternsMenu, &QAction::triggered, this, &Fhex::on_menu_open_patterns_click);

    connect(openTextViewer, &QAction::triggered, this, &Fhex::on_menu_open_text_viewer_click);
    connect(gotoOffset, &QAction::triggered, this, &Fhex::on_menu_goto_offset_click);
    connect(diffFile, &QAction::triggered, this, &Fhex::on_menu_file_diff_click);
    connect(openFile, &QAction::triggered, this, &Fhex::on_menu_file_open_click);
    connect(chunkOpenFile, &QAction::triggered, this, &Fhex::on_menu_file_chunk_open_click);
    connect(saveFile, &QAction::triggered, this, &Fhex::on_menu_file_save_click);
    connect(saveNewFile, &QAction::triggered, this, &Fhex::on_menu_file_save_as_click);
    connect(openNewWindow, &QAction::triggered, this, &Fhex::on_menu_file_new_window_click);
    connect(quit, &QAction::triggered, this, &Fhex::on_menu_file_quit_click);
    connect(find, &QAction::triggered, this, &Fhex::on_menu_find_click);
    connect(convert, &QAction::triggered, this, &Fhex::on_menu_convert_bytes_click);

    QMenu *tools;
    tools = menuBar()->addMenu("&Tools");
    QAction *hexDec = new QAction(QIcon::fromTheme("gtk-convert"), "&Hex<->Dec", this);
    hexDec->setShortcut(QKeySequence(Qt::Key_F2));
    tools->addAction(hexDec);
    QAction *escapeHex = new QAction("&Escape Hex Bytes", this);
    escapeHex->setShortcut(QKeySequence(Qt::Key_F3));
    tools->addAction(escapeHex);
    QAction *fasm = new QAction(QIcon::fromTheme("map-flat"), "&Assembler/Disassembler", this);
    fasm->setShortcut(QKeySequence(Qt::Key_F4));
    tools->addAction(fasm);

    connect(hexDec, &QAction::triggered, this, &Fhex::on_menu_hex_dec_converter_click);
    connect(escapeHex, &QAction::triggered, this, &Fhex::on_menu_escape_hex_click);
    connect(fasm, &QAction::triggered, this, &Fhex::on_menu_fasm_click);

    QMenu *help;
    help = menuBar()->addMenu("&Help");
    QAction *about = new QAction("&About", this);
    help->addAction(about);

    connect(about, &QAction::triggered, this, &Fhex::on_menu_about_click);

    /** End Menu Initialization **/

    QGridLayout *gridLayout = new QGridLayout;

    this->setAcceptDrops(true);

    qhex = new QHexEdit(this);
    qhex->setMinimumWidth(600);
    qhex->setStyleSheet("QHexEdit { background-color: #17120f; color: #ebe5e1; }");
    qhex->setAddressAreaColor(color_dark_gray);
    qhex->setSelectionColor(color_dark_yellow);
    qhex->setHighlightingColor(color_dark_violet);

    gridLayout->addWidget(qhex, 1, 0, 1, 2);

    connect(this->qhex, &QHexEdit::handle_keyPressEvent, this, &Fhex::keyPressEvent);
    connect(this->qhex, &QHexEdit::handle_mouseClick, this, &Fhex::on_editor_mouse_click);
    connect(this->qhex, &QHexEdit::handle_mouseMove, this, &Fhex::on_editor_mouse_move);
    connect(this->qhex->verticalScrollBar(), &QScrollBar::valueChanged, this, &Fhex::on_vertical_scrollbar_change);
    connect(this->qhex->horizontalScrollBar(), &QScrollBar::valueChanged, this, &Fhex::on_horizontal_scrollbar_change);


    QChart *binChart = new QChart();
    binChart->legend()->hide();
    binChart->createDefaultAxes();
    binChart->setTitle("Binary Chart - Byte Values View");
    binChart->setTheme(QChart::ChartThemeDark);
    binChart->setBackgroundBrush(QBrush(QRgb(CHART_BACKGROUND_COLOR)));
    binChart->setMaximumHeight(500);
    this->binChartView = new QChartView(binChart);
    binChartView->setRenderHint(QPainter::Antialiasing);
    binChartView->setVisible(false);

    gridLayout->addWidget(binChartView, 0, 0, 1, 2);

    this->progressBar = new QProgressBar(this);
    this->progressBar->setRange(0, 100);
    this->progressBar->setTextVisible(false);
    this->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #c22a19; border: 1px solid #912114;}");
    this->progressBar->setFixedHeight(3);
    this->progressBar->setVisible(false);
    gridLayout->addWidget(progressBar, 2, 0, 1, 2);

    gridLayout->addWidget(&this->statusBar, 4, 0);
    gridLayout->addWidget(&this->offsetBar, 4, 1, Qt::AlignRight);

    searchBox = new QFrame(this);
    QFormLayout *searchBoxLayout = new QFormLayout(searchBox);
    QHBoxLayout *firstRow = new QHBoxLayout(searchBox);
    QHBoxLayout *secondRow = new QHBoxLayout(searchBox);
    searchButton = new QPushButton("Find next");
    QPushButton *backSearchButton = new QPushButton("Find previous");
    replaceButton = new QPushButton("Replace");
    QPushButton *replaceAllButton = new QPushButton("Replace All");
    connect(backSearchButton, &QPushButton::clicked, this, &Fhex::on_back_search_button_click);
    connect(searchButton, &QPushButton::clicked, this, &Fhex::on_search_button_click);
    connect(replaceButton, &QPushButton::clicked, this, &Fhex::on_replace_button_click);
    connect(replaceAllButton, &QPushButton::clicked, this, &Fhex::on_replace_all_button_click);
    backSearchButton->setFixedWidth(100);
    backSearchButton->setToolTip("Find the previous occurrence starting from the cursor position");
    searchButton->setFixedWidth(80);
    this->searchButton->setToolTip("Find the next occurrence starting from the cursor position");
    replaceButton->setFixedWidth(60);
    this->replaceButton->setToolTip("Replace the next occurrence starting from the cursor position");
    replaceAllButton->setFixedWidth(80);
    replaceAllButton->setToolTip("Replace all the occurences from the beginning");
    this->searchText = new QPlainTextEdit(searchBox);
    this->replaceText = new QPlainTextEdit(searchBox);
    searchText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    searchText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->searchText->setObjectName("searchText");
    this->searchText->installEventFilter(this);
    replaceText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    replaceText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->replaceText->setObjectName("replaceText");
    this->replaceText->installEventFilter(this);
    this->searchFormatOption = new QComboBox(this);
    searchFormatOption->addItem("UTF-8");
    //TODO: formatOption->addItem("UTF-16");
    searchFormatOption->addItem("HEX");
    searchFormatOption->setFixedWidth(80);
    replaceText->setStyleSheet("QPlainTextEdit { margin-left: 90px; }");
    firstRow->addWidget(searchFormatOption);
    firstRow->addWidget(searchText);
    this->regexCheckBox.setText("regex");
    this->regexCheckBox.setChecked(false);
    this->regexCheckBox.setFixedWidth(60);
    firstRow->addWidget(&regexCheckBox);
    firstRow->addWidget(searchButton);
    firstRow->addWidget(backSearchButton);
    secondRow->addWidget(replaceText);
    secondRow->addWidget(replaceButton);
    secondRow->addWidget(replaceAllButton);
    searchBoxLayout->addRow(firstRow);
    searchBoxLayout->addRow(secondRow);
    searchBox->setLayout(searchBoxLayout);
    searchBox->setFixedHeight(100);
    searchBox->setMinimumWidth(this->width());
    searchText->setFixedWidth(searchBox->width() / 2);
    replaceText->setFixedWidth(searchText->width());
    searchBox->setObjectName("container");
    searchBox->setStyleSheet("QFrame#container { border: 1px solid #bbbbbb; padding: 0px; margin: 0px;}");
    searchBox->setVisible(false);

    gridLayout->addWidget(searchBox, 3, 0, 1, 2, Qt::AlignLeft);

    convertBox = new QFrame(this);
    QVBoxLayout *convertBoxLayout = new QVBoxLayout(convertBox);
    QPushButton *convertButton = new QPushButton("Convert Selection");
    convertBoxLayout->addWidget(convertButton);
    this->convertLabel.setTextInteractionFlags(Qt::TextSelectableByMouse);
    convertBoxLayout->addWidget(&this->convertLabel);
    convertBox->setLayout(convertBoxLayout);
    connect(convertButton, &QPushButton::clicked, this, &Fhex::on_convert_button_click);
    convertBox->setVisible(false);

    gridLayout->addWidget(convertBox, 1, 3);

    listOffsets = new QListWidget(this);
    connect(listOffsets, &QListWidget::itemClicked, this, &Fhex::on_list_offset_item_click);
    listOffsets->setFixedWidth(200);
    listOffsets->setStyleSheet("QListWidget { background-color: #0a0600; color: #e3e3e3; font-size: 14px;};");
    listOffsets->setVisible(false);
    gridLayout->addWidget(listOffsets, 1, 3);

    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(gridLayout);

    this->statusBar.setText("Fhex loaded");
    this->setCentralWidget(mainWidget);
    this->fasm = nullptr;

    //If a filepath was passed as argument, open it
    if (filepath != "") {
        this->loadFile(filepath);
        this->loadTables();

        // IF PATTERNS ENABLED
        if (patternsEnabled) {
            findPatterns();
        }
    }

}

Fhex::~Fhex()
{
    delete this->hexEditor;
    if (this->fasm != nullptr) {
        delete this->fasm;
    }
}

void Fhex::on_menu_about_click() {
    QMainWindow *newWindow = new QMainWindow(this);
    newWindow->setWindowTitle("About");
    newWindow->setWindowFlags(Qt::Dialog | Qt::Drawer);
    newWindow->layout()->setSizeConstraint( QLayout::SetFixedSize );
    newWindow->setMinimumWidth(400);
    newWindow->setMinimumHeight(200);
    QLabel *appName = new QLabel("Fhex");
    QLabel *version = new QLabel(this->version.c_str());
    QLabel *title = new QLabel("Full-featured Hex Editor");
    QLabel *desc = new QLabel("Free and Open Source");
    QLabel *author = new QLabel("URL: <a href=\"https://github.com/echo-devim/fhex\">https://github.com/echo-devim/fhex</a>");
    QLabel *libraries = new QLabel("Libraries:");
    QLabel *libCapstone = new QLabel("Disassembler: <a href=\"https://www.capstone-engine.org\">https://www.capstone-engine.org</a>");
    QLabel *libKeystone = new QLabel("Assembler: <a href=\"https://www.keystone-engine.org\">https://www.keystone-engine.org</a>");
    QLabel *libJSON = new QLabel("JSON: <a href=\"https://json.nlohmann.me\">https://json.nlohmann.me</a>");
    appName->setStyleSheet("QLabel { font-size: 25px; }");
    libraries->setStyleSheet("QLabel { font-weight: bold; }");
    appName->setFixedWidth(newWindow->width());
    title->setFixedWidth(newWindow->width());
    desc->setFixedWidth(newWindow->width());
    author->setFixedWidth(newWindow->width());
    author->setOpenExternalLinks(true);
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(appName, 0, 0, 1, 0, Qt::AlignLeft);
    grid->addWidget(version, 1, 0, 1, 0, Qt::AlignLeft);
    grid->addWidget(title, 2, 0, 1, 0, Qt::AlignLeft);
    grid->addWidget(desc, 3, 0, 1, 0, Qt::AlignLeft);
    grid->addWidget(author, 4, 0, 1, 0, Qt::AlignLeft);
    grid->addWidget(libraries, 5, 0, 1, 0, Qt::AlignLeft);
    grid->addWidget(libCapstone, 6, 0, 1, 0, Qt::AlignLeft);
    grid->addWidget(libKeystone, 7, 0, 1, 0, Qt::AlignLeft);
    grid->addWidget(libJSON, 8, 0, 1, 0, Qt::AlignLeft);
    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(grid);
    newWindow->setCentralWidget(mainWidget);
    newWindow->show();
}

void Fhex::on_menu_fasm_click() {
    if (fasm != nullptr) {
        delete fasm;
        this->fasm = nullptr;
    }
    fasm = new Fasm(this->qhex->selectedData());
    fasm->show();
}

void Fhex::on_menu_offset_list_click() {
    this->listOffsets->setVisible(!this->listOffsets->isVisible());
}

void Fhex::on_list_offset_item_click(QListWidgetItem *item) {
    QString text = item->text();
    text.replace(" ", "");
    text.replace("0x", "");
    if (!text.isEmpty()) {
        qint64 offset = text.toLongLong(nullptr, 16);
        if (offset <= static_cast<long long>(this->hexEditor->loadedFileSize)) {
            this->qhex->setCursorPosition(offset * 2);
            this->qhex->ensureVisible();
        } else {
            this->statusBar.setText("Error: Out-of-bound offset specified");
        }
    }
}

void Fhex::backgroundLoadTables(long index) {
    if (this->hexEditor->loadedFileSize > 2147483647) {
        this->qhex->setData(QString(this->hexEditor->getCurrentPath().c_str()));
    } else {
        this->qhex->setData(reinterpret_cast<const char*>(this->hexEditor->getCurrentData().data()), this->hexEditor->loadedFileSize);
    }
    this->initialized_tables = true;
}

void Fhex::loadTables(long index) {
    this->initialized_tables = false;

    std::thread t(&Fhex::backgroundLoadTables, this, index);
    t.detach();

}


void Fhex::updateOffsetBar() {
    qint64 offset = this->currentCursorPos;
    this->offsetBar.setText("Offset: 0x" + QString::number(offset, 16) + " (" + QString::number(offset) + ") | "
                            + "File Size: " + QString::number(this->hexEditor->fileSize / 1024.0, 'f', 2) + " KB");
}

void Fhex::updateOffsetBarWithSelection() {
    pair<qint64,qint64> offsets = this->qhex->selectedOffsets();
    offsets.second = offsets.second - 1;
    if (offsets.second - offsets.first > 0) {
        this->offsetBar.setText("First Offset: 0x" + QString::number(offsets.first, 16) + " (" + QString::number(offsets.first) + ") | "
                                + "Last Offset: 0x" + QString::number(offsets.second, 16) + " (" + QString::number(offsets.second) + ") | "
                                + "Selected bytes: " + QString::number(offsets.second - offsets.first + 1) + " | "
                                + "File Size: " + QString::number(this->hexEditor->fileSize / 1024.0, 'f', 2) + " KB");
    }
}
void Fhex::on_editor_mouse_click() {
    currentCursorPos = this->qhex->cursorPosition() / 2;
    if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
        this->qhex->setSelection(lastCursorPos, this->qhex->cursorPosition() / 2);
        updateOffsetBarWithSelection();
    } else {
        updateOffsetBar();
        lastCursorPos = currentCursorPos;
    }
}

void Fhex::on_editor_mouse_move() {
    updateOffsetBarWithSelection();
}


void Fhex::keyPressEvent(QKeyEvent *event) {
    if(event->type() == QKeyEvent::KeyPress) {
        if ((event->key() == Qt::Key_Space) && QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
            pair<qint64,qint64> offsets = this->qhex->selectedOffsets();
            QApplication::clipboard()->setText(QString::fromWCharArray(this->hexEditor->getCurrentDataAsWString(offsets.first, offsets.second - offsets.first).c_str()));
        } else if (event->key() == Qt::Key_F5) {
            this->loadFile(this->hexEditor->getCurrentPath().c_str());
            this->loadTables();
        }
        updateOffsetBar();
        updateOffsetBarWithSelection();
    }
}

/*
 *  Toggle patterns display On/Off
 */

void Fhex::on_menu_toggle_patterns_click() {
    if (patternsEnabled){
        patternsEnabled = false;
    } else {
        patternsEnabled = true;
    }

    // Check if there is a file and is fully loaded
    // Then reload with pattern applied (probably there is a better way of doing this,
    // without having to reload the file, but due to if pattern was previously applied
    // I think HexEditor has to be created accordingly to use of pattern), easiest
    // way is just to reload the file and parse accordingly :)

    if (this->hexEditor->loadedFileSize > 0 && this->hexEditor->isFileLoaded()) {
        QString path(this->hexEditor->getCurrentPath().c_str());
        if (patternsEnabled){
            this->hexEditor = new HexEditor(patternsFile);
            this->loadFile(path);
            findPatterns();
        } else {
            this->hexEditor = new HexEditor();
            this->listOffsets->clear();
            this->loadFile(path);
        }
    }
}

/*
 * OPENS FILE SELECTOR FOR CHOOSING PATTERNS FILE TO USE
 */

void Fhex::on_menu_open_patterns_click() {
    QString path(this->hexEditor->getCurrentPath().c_str());
    QString patternsPath = QFileDialog::getOpenFileName(this,
        tr("Open File"), path,
        tr("JSON Files (*.json)"));
    if (patternsPath != "") {
        patternsFile = patternsPath.toStdString();

        /*
         * RELOAD FILE WITH PATTERNS FILE IF ENABLED,
         * PARSE AND PATTERN PRINT LABELS
         */

        if (patternsEnabled) {
            this->hexEditor = new HexEditor(patternsFile);
            this->loadFile(path);
            findPatterns();
        }
    }
}

void Fhex::findPatterns() {
    clearFloatingLabels();
    this->listOffsets->clear();
    this->statusBar.setText("Searching patterns..");
    this->statusBar.repaint();
    unsigned long patterns = 0;
    vector<Match *> matches = this->hexEditor->findPatterns();
    size_t size = matches.size();
    for (Match *m : matches) {
        if (patterns > MAX_PATTERN_RESULTS) {
            break;
        }
        // render highlight area
        QString style("QLabel { color: #fbfbfb; padding: 2px; background-color: ");
        style += m->color.c_str();
        style += " };";

        /*
         * TODO: CAPTURE WINDOW RESIZE EVENT AND IMPLEMENT LOGIC
         * TO SHOW PATTERNS IF THEY FIT ON SCREEN
         */

        //Show comments only if the windows is maximized, otherwise probably we don't have enough space
        addFloatingLabel(m->index, m->length, m->message.c_str(), style, this->windowState().testFlag(Qt::WindowMaximized));
        this->listOffsets->addItem("0x" + QString::number(m->index, 16));
        delete m;
        patterns++;
    }
    if (size > MAX_PATTERN_RESULTS) {
        this->statusBar.setText("Found " + QString::number(size) + " patterns. Limit exceeded, showing labels only for first " + QString::number(MAX_PATTERN_RESULTS));
    } else {
        this->statusBar.setText("Found " + QString::number(patterns) + " patterns");
    }
    this->listOffsets->setVisible(true);
}

void Fhex::on_menu_find_click() {
    this->searchBox->setVisible(!this->searchBox->isVisible());
}

void Fhex::clearFloatingLabels() {
    //Clear all floating labels if present
    for (QLabel *label : this->floatingLabels) {
        label->close();
    }
    this->floatingLabels.clear();
}

bool Fhex::loadFile(QString path, unsigned long start, unsigned long offset) {
    this->qhex->clear();
    QFileInfo finfo = QFileInfo(path);
    if (offset == 0)
        offset = finfo.size();
    if (offset > FILE_SIZE_LIMIT) {
        string warning = "Cannot load the entire file into memory, please select the file chunk to view. Maximum amount of allowed MB in memory: " + to_string((FILE_SIZE_LIMIT/(1024*1024)));
        cerr << warning << endl;
        QMessageBox msgBox;
        msgBox.setText(QString(warning.c_str()));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.setWindowTitle(this->windowTitle());
        msgBox.setWindowIcon(this->windowIcon());
        msgBox.exec();
        //chunkOpenFile will call loadFile again
        this->chunkOpenFile(path);
        return false;
    }
    this->progressBar->setVisible(true);
    this->progressBar->setValue(0);
    this->clearFloatingLabels();
    this->statusBar.setText("Loading " + path);
    auto t1 = std::chrono::high_resolution_clock::now();
    bool res = this->hexEditor->loadFileAsync(path.toStdString(), start, offset);
    while(!this->hexEditor->isFileLoaded()) {
        int val = static_cast<int>(this->hexEditor->bytesRead * 100 / this->hexEditor->loadedFileSize);
        this->progressBar->setValue(val);
        this->statusBar.setText("Loading " + QString::number(val) + "%");
        this->repaint();
        this->app->processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    this->progressBar->setVisible(false);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    if (res) {
        QString portion = "";
        if (this->hexEditor->loadedFileSize < this->hexEditor->fileSize) {
            portion = QString::number(this->hexEditor->loadedFileSize / 1024) + " KB of ";
        }
        this->statusBar.setText("Loaded (" + portion + QString::number(this->hexEditor->fileSize / 1024) + " KB) in " + QString::number(duration / 1000.) + "s");
        this->setWindowTitle("Fhex - " + QString(this->hexEditor->getCurrentPath().c_str()));
        loadBinChart();
    } else {
        this->statusBar.setText("Error while opening " + path);
    }
    return res;
}

void Fhex::loadBinChart() {
    QLineSeries *series = new QLineSeries();
    unsigned long step = 1;
    if (this->hexEditor->loadedFileSize > CHART_DENSITY)
        step = this->hexEditor->loadedFileSize / CHART_DENSITY;
    for (unsigned long offset = 0; offset < this->hexEditor->loadedFileSize; offset += step) {
        series->append(offset, this->hexEditor->getCurrentData()[offset]);
    }

    QPen pen(QRgb(CHART_LINE_COLOR));
    pen.setWidth(2);
    series->setPen(pen);

    connect(series, &QLineSeries::clicked, this, &Fhex::on_binchart_click);

    binChartView->chart()->removeAllSeries();
    binChartView->chart()->addSeries(series);
    binChartView->chart()->createDefaultAxes();
}

void Fhex::on_binchart_click(const QPointF &p) {
    this->qhex->setCursorPosition(static_cast<qint64>(p.x()) * 2);
    this->qhex->ensureVisible();
}

void Fhex::on_menu_file_chunk_open_click() {
    this->chunkOpenFile();
}

void Fhex::chunkOpenFile(QString fpath) {
    unsigned long start = 0;
    unsigned long offset = 0;
    QString path;
    QDialog *chunkWindow = new QDialog(this);
    chunkWindow->setWindowIcon(this->windowIcon());
    chunkWindow->setWindowTitle("Fhex - Chunk loader");
    chunkWindow->setFixedSize(250, 300);
    QVBoxLayout *mainLayout =new QVBoxLayout(chunkWindow);
    QLabel *labelWarning = new QLabel("The save operation saves the whole file, including the other chunks not loaded.", chunkWindow);
    labelWarning->setStyleSheet("font-style: italic;");
    labelWarning->setWordWrap(true);
    labelWarning->setFixedHeight(50);
    labelWarning->setContentsMargins(0,0,0,10);
    QHBoxLayout *fileLayout = new QHBoxLayout(chunkWindow);
    QLabel *labelFile = new QLabel("File: ", chunkWindow);
    QLineEdit *filepath = new QLineEdit(chunkWindow);
    filepath->setText(fpath);
    QPushButton *btnOpen = new QPushButton("Select", chunkWindow);
    btnOpen->setFixedWidth(60);
    fileLayout->addWidget(filepath);
    fileLayout->addWidget(btnOpen);
    QLabel *labelStart = new QLabel("Start offset: ", chunkWindow);
    labelStart->setFixedHeight(20);
    QLabel *labelOffset = new QLabel("Length offset:", chunkWindow);
    labelOffset->setFixedHeight(20);
    QSpinBox *startOffset = new QSpinBox(chunkWindow);
    startOffset->setFixedHeight(30);
    startOffset->setMinimum(0);
    startOffset->setMaximum(FILE_SIZE_LIMIT);
    QSpinBox *lengthOffset = new QSpinBox(chunkWindow);
    lengthOffset->setFixedHeight(30);
    lengthOffset->setMinimum(1);
    lengthOffset->setMaximum(FILE_SIZE_LIMIT);
    QPushButton *btnLoad = new QPushButton("Load", chunkWindow);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(chunkWindow);
    QWidget *container = new QWidget(chunkWindow);
    container->setLayout(fileLayout);
    buttonsLayout->addWidget(btnLoad);
    mainLayout->addWidget(labelWarning);
    mainLayout->addWidget(labelFile);
    mainLayout->addWidget(container);
    mainLayout->addWidget(labelStart);
    mainLayout->addWidget(startOffset);
    mainLayout->addWidget(labelOffset);
    mainLayout->addWidget(lengthOffset);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->setSpacing(4);
    connect(btnLoad, &QPushButton::clicked, [chunkWindow, startOffset, lengthOffset, &start, &offset]()
    {
        start = startOffset->value();
        offset = lengthOffset->value();
        chunkWindow->close();
    });
    connect(btnOpen, &QPushButton::clicked, [this, filepath]()
    {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), filepath->text(),
            tr("All Files (*)"));
        if (fileName != "") {
            filepath->setText(fileName);
        }
    });
    chunkWindow->setLayout(mainLayout);
    chunkWindow->exec();
    path = filepath->text();
    this->loadFile(path, start, offset);
    this->loadTables();
}

void Fhex::on_menu_file_open_click() {
    QString path(this->hexEditor->getCurrentPath().c_str());
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), path,
        tr("All Files (*)"));
    if (fileName != "") {
        this->loadFile(fileName);
        this->loadTables();

        // IF PATTERNS ENABLED
        if (patternsEnabled) {
            findPatterns();
        }
    }
}

void Fhex::on_menu_file_diff_click() {
    QString path(this->hexEditor->getCurrentPath().c_str());
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), path,
        tr("All Files (*)"));
    if (fileName != "") {
        compare(fileName);
    }
}

void Fhex::compare(QString filename) {

    // Clean-up display
    this->clearFloatingLabels();
    this->listOffsets->clear();

    this->hexEditor->bytesRead = 0;
    this->progressBar->setVisible(true);
    this->progressBar->setValue(0);
    this->statusBar.setText("Comparing file.. please wait");

    future<vector<pair<unsigned long, uint8_t>>> fut_res = async([this, filename]()
    {
        HexEditor newHexEditor(patternsFile);
        newHexEditor.loadFileAsync(filename.toStdString());
        while(!newHexEditor.isFileLoaded()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return this->hexEditor->compareTo(newHexEditor);
    });

    while (fut_res.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready) {
        int val = (this->hexEditor->bytesRead * 100) / this->hexEditor->loadedFileSize;
        this->progressBar->setValue(val);
        this->repaint();
        this->app->processEvents();
    }

    this->progressBar->setVisible(false);

    vector<pair<unsigned long, uint8_t>> res = fut_res.get();
    unsigned long changes = res.size();
    unsigned long start_offset = 0;
    unsigned long offset = 0;
    int limit = MAX_DIFF_BYTES; //Show maximum 3000 different bytes
    QByteArray diff_bytes;
    for (pair<unsigned long, uint8_t> p : res) {
        if (offset == 0) {
            start_offset = p.first;
            offset = start_offset;
            listOffsets->addItem("0x" + QString::number(start_offset, 16));
        }
        if (p.first - offset > 0) {
            if (limit > 0)
                addFloatingLabel(start_offset, static_cast<int>(diff_bytes.size()), "Compared file:\r\n" + diff_bytes.toHex(' ') + "\r\n-----------\r\n" + diff_bytes, DIFF_STYLE);
            diff_bytes.clear();
            offset = 0;
        } else {
            offset++;
        }
        diff_bytes.push_back(static_cast<char>(p.second));
        if (limit > 0)
            limit--;
    }
    if (diff_bytes.size() > 0) {
        if (limit > 0)
            addFloatingLabel(start_offset, static_cast<int>(diff_bytes.size()), "After:\r\n" + diff_bytes.toHex(' ') + "\r\n-----------\r\n" + diff_bytes, DIFF_STYLE);
        diff_bytes.clear();
    }

    if (changes == 0)
        this->statusBar.setText("Files are equal");
    else {
        this->statusBar.setText("Found " + QString::number(changes) + " different bytes");
        listOffsets->setVisible(true);
    }

    if (limit == 0) {
        QMessageBox msgBox;
        msgBox.setText("The output displays only the first " + QString::number(MAX_DIFF_BYTES) + " different bytes for performance reasons.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.setWindowTitle(this->windowTitle());
        msgBox.setWindowIcon(this->windowIcon());
        msgBox.exec();
    }
}

void Fhex::on_menu_file_save_click() {
    this->statusBar.setText("Saving file..");
    if (this->qhex->isModified()) {
        saveDataToFile(this->hexEditor->getCurrentPath());
    } else {
        this->statusBar.setText("No changes were made");
    }
}

void Fhex::on_search_button_click() {
    qint64 start = this->qhex->cursorPosition() / 2;
    if (this->searchFormatOption->currentText() == "UTF-8") {
        qint64 res = this->qhex->indexOf(this->searchText->toPlainText().toUtf8(), start, this->regexCheckBox.isChecked());
        if (res < 0) {
            this->statusBar.setText("No match found");
        } else {
            this->statusBar.setText("Found match at 0x" + QString::number(res, 16));
        }
    } else if (this->searchFormatOption->currentText() == "HEX") {
        QString searchedText = this->searchText->toPlainText().toLower().replace(" ", "");
        qint64 res = this->qhex->indexOf(QByteArray::fromHex(searchedText.toLatin1()), start, this->regexCheckBox.isChecked());
        if (res < 0) {
            this->statusBar.setText("No match found");
        } else {
            this->statusBar.setText("Found match at 0x" + QString::number(res, 16));
        }
    }
}

void Fhex::on_back_search_button_click() {
    //Actually regex are not supported in backward search
    if (this->regexCheckBox.isChecked()) {
        this->regexCheckBox.setChecked(false);
    }

    qint64 start = this->qhex->cursorPosition() / 2;
    if (this->searchFormatOption->currentText() == "UTF-8") {
        qint64 res = this->qhex->lastIndexOf(this->searchText->toPlainText().toUtf8(), start);
        if (res < 0) {
            this->statusBar.setText("No match found");
        } else {
            this->statusBar.setText("Found match at 0x" + QString::number(res, 16));
            this->qhex->setCursorPosition(res);
        }
    } else if (this->searchFormatOption->currentText() == "HEX") {
        QString searchedText = this->searchText->toPlainText().toLower().replace(" ", "");
        qint64 res = this->qhex->lastIndexOf(QByteArray::fromHex(searchedText.toLatin1()), start);
        if (res < 0) {
            this->statusBar.setText("No match found");
        } else {
            this->statusBar.setText("Found match at 0x" + QString::number(res, 16));
            this->qhex->setCursorPosition(res);
        }
    }
}

qint64 Fhex::replaceBytes(QString searchText, QString replaceText, bool isHex) {
    qint64 start = this->qhex->cursorPosition() / 2;
    if (start > 0)
        start++;
    qint64 res = -1;
    if (!isHex) { // Plain text case
        res = this->qhex->indexOf(searchText.toUtf8(), start, this->regexCheckBox.isChecked());
        if (res >= 0) {
            this->qhex->replace(res, replaceText.length(), replaceText.toUtf8());
        }
    } else {
        QString sText = searchText.toLower().replace(" ", "");
        QString rText = replaceText.toLower().replace(" ", "");
        res = this->qhex->indexOf(QByteArray::fromHex(sText.toLatin1()), start, this->regexCheckBox.isChecked());
        if (res >= 0) {
            this->qhex->replace(res, rText.length(), QByteArray::fromHex(rText.toLatin1()));
        }
    }
    return res;
}

void Fhex::on_replace_button_click() {
    bool isHex = (this->searchFormatOption->currentText() == "HEX");
    qint64 res = replaceBytes(this->searchText->toPlainText(), this->replaceText->toPlainText(), isHex);
    if (res < 0) {
        this->statusBar.setText("No match found");
    } else {
        this->statusBar.setText("Found match at 0x" + QString::number(res, 16));
        this->qhex->setCursorPosition(res);
    }
}

void Fhex::on_replace_all_button_click() {
    this->qhex->setCursorPosition(0);
    this->statusBar.setText("Replacing all occurences..please wait");
    this->statusBar.repaint();
    long matches = 0;
    qint64 res = 1;
    bool isHex = (this->searchFormatOption->currentText() == "HEX");
    this->progressBar->setVisible(true);
    this->progressBar->setValue(0);
    while (res >= 0) {
        res = replaceBytes(this->searchText->toPlainText(), this->replaceText->toPlainText(), isHex);
        if (res >= 0) {
            matches++;
            this->statusBar.setText("Current matches: " + QString::number(matches));
            this->progressBar->setValue(static_cast<int>(static_cast<unsigned long>(res)*100 / this->hexEditor->loadedFileSize));
            this->statusBar.repaint();
            this->qhex->setCursorPosition(res + 1);
        }
    }
    this->progressBar->setVisible(false);

    if (matches == 0)
        this->statusBar.setText("No match found");
    else
        this->statusBar.setText("Replaced " + QString::number(matches) + " occurences");
}

void Fhex::on_menu_file_save_as_click() {
    this->statusBar.setText("Saving file..");
    QString path(this->hexEditor->getCurrentPath().c_str());
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save File"), path,
        tr("All Files (*)"));
    if (fileName != "") {
        saveDataToFile(fileName.toStdString());
        this->statusBar.setText("File saved as " + fileName);
    }
}

void Fhex::saveDataToFile(string path) {
    this->hexEditor->getCurrentData().clear();
    this->hexEditor->getCurrentData().shrink_to_fit();
    QByteArray datacopy(this->qhex->data());
    this->hexEditor->getCurrentData().insert(this->hexEditor->getCurrentData().begin(), datacopy.begin(), datacopy.end());
    this->hexEditor->loadedFileSize = this->qhex->data().size();

    this->progressBar->setVisible(true);
    this->progressBar->setValue(0);
    this->statusBar.setText("Saving " + QString(path.c_str()));
    bool res = this->hexEditor->saveFileAsync(path);
    while(!this->hexEditor->isFileSaved()) {
        int val = static_cast<int>(this->hexEditor->bytesSaved * 100 / this->hexEditor->fileSize);
        this->progressBar->setValue(val);
        this->statusBar.setText("Saving " + QString::number(val) + "%");
        this->repaint();
        this->app->processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    this->progressBar->setVisible(false);
    if (res) {
        this->statusBar.setText("Saved " + QString(path.c_str()));
    } else {
        this->statusBar.setText("Error while saving " + QString(path.c_str()));
    }
}

void Fhex::on_menu_convert_bytes_click() {
    this->convertLabel.setText("<html><br><i>Select one or more bytes,<br>then press the button</i></html>");
    this->convertBox->setVisible(!this->convertBox->isVisible());
}

void Fhex::on_convert_button_click() {
    QString out = "";
    QString data = this->qhex->selectedData();
    out += "<html><i>Big Endian</i><hr>Decimal Integer:<br><b>" + QString::number(data.toInt(nullptr, 16)) + "</b>";
    out += "<hr>Decimal Long:<br><b>" + QString::number(data.toLong(nullptr, 16)) + "</b>";
    out += "<hr>Decimal Unsigned Long:<br><b>" + QString::number(data.toULong(nullptr, 16)) + "</b>";
    QString revData = "";
    for (int i=data.length()-2; i >= 0; i-=2) {
        revData += data.mid(i, 2);
    }
    out += "<hr><br><i>Little Endian</i><hr>Decimal Integer:<br><b>" + QString::number(revData.toUInt(nullptr, 16)) + "</b>";
    out += "<hr>Decimal Long:<br><b>" + QString::number(revData.toLong(nullptr, 16)) + "</b>";
    out += "<hr>Decimal Unsigned Long:<br><b>" + QString::number(revData.toULong(nullptr, 16)) + "</b></html>";
    this->convertLabel.setText(out);
}

void Fhex::dropEvent(QDropEvent *event) {
    if (event->mimeData()->urls().size() > 1) {
        //More than one file, compare them
        //Clean-up
        this->listOffsets->clear();
        // NEW HEXEDITOR WITHOUT PATTERNS
        this->hexEditor = new HexEditor();
        //Next files
        const QUrl url = event->mimeData()->urls().at(0);
        QString fileName = url.toLocalFile();
        this->loadFile(fileName);
        this->loadTables();

        this->compare(event->mimeData()->urls().at(1).toLocalFile());
    } else {
        //One file
        if (patternsEnabled) {
            // NEW HEXEDITOR WITH PATTERNS ENABLED
            this->hexEditor = new HexEditor(patternsFile);
        } else {
            // NEW HEXEDITOR WITH PATTERNS NOT ENABLED
            this->hexEditor = new HexEditor();
            // If previously there was a file loaded as there is no way to see if
            // a comparison was done previously
            // clear & hide offsetlist as there are no expected patterns to be shown
            if (this->hexEditor->isFileLoaded()) {
                this->listOffsets->clear();
                this->listOffsets->setVisible(false);
            }
        }

        const QUrl url = event->mimeData()->urls().at(0);
        QString fileName = url.toLocalFile();
        this->loadFile(fileName);
        this->loadTables();

        // ADD PATTERNS IF ENABLED

        if (patternsEnabled) {
            findPatterns();
        }
    }
}

void Fhex::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void Fhex::on_menu_file_new_window_click() {
    Fhex *newFhex = new Fhex(nullptr, this->app);
    newFhex->show();
}

/*
 * SAVE WINDOW SIZE AND POSITION SETTINGS
 */

void Fhex::saveSettings(string filePath){
    jsettings["WindowSettings"]["width"] = this->size().width();
    jsettings["WindowSettings"]["height"] = this->size().height();
    QRect windowPosition = geometry();
    jsettings["WindowSettings"]["x_position"] = windowPosition.x();
    jsettings["WindowSettings"]["y_position"] = windowPosition.y();
    jsettings["WindowSettings"]["maximized"] = this->windowState().testFlag(Qt::WindowMaximized);
    jsettings["Patterns"]["enabled"] = patternsEnabled;
    jsettings["Patterns"]["file"] = patternsFile;

    // Merge changes
    jconfig.merge_patch(jsettings);
    // Create directory if needed
#ifdef WINDOWS
    mkdir(settingsPath.c_str());
#else
    mkdir(settingsPath.c_str(), 0777);
#endif
    // Save Window size & position
    std::ofstream configFile;
    try {
        configFile.open(filePath,std::fstream::binary | std::fstream::trunc | std::fstream::out);
    } catch (exception &e) {
        cerr << "Exception occurred while creating json configuration file:" << endl << e.what() << endl;
    }
    // Write values
    try {
        configFile << jconfig;
        configFile.close();
    } catch (exception &e) {
        cerr << "Exception occurred while writting json configuration file:" << endl << e.what() << endl;
    }
}

void Fhex::on_menu_file_quit_click() {
    saveSettings(settingsFile);
    QApplication::quit();
}

/*
 * CAPTURE 'X' BUTTON EVENT,
 * NOT EVERY WM PROVIDES AN X BUTTON TO CLOSE
 * WE NEED TO SAVE WINDOW SETTINGS UPON TEAR DOWN
*/

void Fhex::closeEvent(QCloseEvent *event)
{
    saveSettings(settingsFile);
    event->accept();
}

void Fhex::on_menu_goto_offset_click() {
    bool ok;
    QString text = QInputDialog::getText(this, "Goto Offset", "Offset:", QLineEdit::Normal, "", &ok);
    text.replace(" ", "");
    text.replace("0x", "");
    if (ok && !text.isEmpty()) {
        qint64 offset = text.toLongLong(nullptr, 16);
        if (offset <= static_cast<long long>(this->hexEditor->loadedFileSize)) {
            this->qhex->setCursorPosition(offset * 2);
            this->qhex->ensureVisible();
        } else {
            this->statusBar.setText("Error: Out-of-bound offset specified");
        }
    }
}

void Fhex::on_menu_open_text_viewer_click() {
    QMainWindow *newWindow = new QMainWindow(this);
    newWindow->setWindowTitle("Fhex - Text Viewer");
    QTextEdit *textEdit = new QTextEdit(newWindow);
    textEdit->setStyleSheet("QTextEdit { background-color: #17120f; color: #ebe5e1; font-size: 16px; }");
    textEdit->setText(QByteArray::fromHex(this->qhex->selectedData().replace("00", "0a").toUtf8()));
    if (textEdit->toPlainText() == "")
        textEdit->setText(this->qhex->data());
    newWindow->setMinimumWidth(this->width());
    newWindow->setMinimumHeight(this->height());
    newWindow->setCentralWidget(textEdit);
    newWindow->show();
}

void Fhex::addFloatingLabel(qint64 offset, int len, QString text, QString style, bool addComment) {
    int columns = this->qhex->bytesPerLine();
    int offsetCol = offset % columns;
    int diff = (offsetCol + len) - columns;
    if (diff > 0) { //The length is bigger than columns
        len = columns - offsetCol; //the label will have a width as long as the end of the row
        addFloatingLabel(offset + len, diff, text, style);
    }
    QPoint p = this->qhex->getOffsetPos(offset);
    QLabel *label = new QLabel(this->qhex);
    if (style == "")
        style = "QLabel { background-color: rgb(150, 150, 150, 50); }";
    label->setStyleSheet(style);
    label->setToolTip(text);
    label->move(p);
    label->resize((this->qhex->getPxCharWidth()*3) * len, this->qhex->getPxCharHeight());
    label->show();
    this->floatingLabels.push_back(label);

    if (addComment) {
        QLabel *commentLabel = new QLabel(this->qhex);
        if (style == "")
            style = "QLabel { background-color: rgb(150, 150, 150, 50); color: #ffffff; }";
        commentLabel->setStyleSheet(style);
        commentLabel->setText(text);
        commentLabel->move(p.x() + (this->width() / 2.5), p.y());
        commentLabel->resize(this->qhex->getPxCharWidth() * text.size(), this->qhex->getPxCharHeight());
        commentLabel->show();
        this->floatingLabels.push_back(commentLabel);
    }

}

void Fhex::on_vertical_scrollbar_change(int value) {
    int step = value - this->prev_vscrollbar_value;
    if (step != 0) {
        //Adjust the position of floating labels on scrolling
        for (QLabel *label : this->floatingLabels) {
            label->move(label->x(), label->y() - (step * label->height()));
        }
        this->prev_vscrollbar_value = value;
    }
}

void Fhex::on_horizontal_scrollbar_change(int value) {
    int step = value - this->prev_hscrollbar_value;
    if (step != 0) {
        //Adjust the position of floating labels on scrolling
        for (QLabel *label : this->floatingLabels) {
            label->move(label->x() - step, label->y());
        }
        this->prev_hscrollbar_value = value;
    }
}

void Fhex::on_menu_new_file_click() {
    // CREATE NEW HexEditor AND CLEAR PATTERNS IF THEY EXIST
    this->hexEditor = new HexEditor();
    this->clearFloatingLabels();
    this->listOffsets->clear();
    this->listOffsets->setVisible(false);
    //Add one null byte
    this->qhex->setData(QByteArray::fromStdString("."));
    this->statusBar.setText("New Hex buffer created");
}

bool Fhex::eventFilter(QObject* o, QEvent* e){
    if (e->type() != QEvent::KeyPress)
        return QObject::eventFilter(o, e);

    QKeyEvent* eventKey = static_cast<QKeyEvent*>(e);
    if ((eventKey->key() == Qt::Key_Enter) || (eventKey->key() == Qt::Key_Return)) {
        this->statusBar.setText("Pressed Enter");
        if (o->objectName() == "searchText")
            this->searchButton->click();
        else if (o->objectName() == "replaceText")
            this->replaceButton->click();
        return true;
    } else {
        return QObject::eventFilter(o, e);
    }
}

void Fhex::on_menu_hex_dec_converter_click() {
    QMainWindow *converterWindow = new QMainWindow(this);
    converterWindow->setWindowIcon(this->windowIcon());
    converterWindow->setWindowTitle("Fhex - Converter");
    converterWindow->setFixedSize(300, 200);
    QVBoxLayout *mainLayout =new QVBoxLayout(converterWindow);
    QLabel *labelDec = new QLabel("Decimal:", converterWindow);
    labelDec->setFixedHeight(20);
    QLabel *labelHex = new QLabel("Hexadecimal:", converterWindow);
    labelHex->setFixedHeight(20);
    QPlainTextEdit *decText = new QPlainTextEdit(converterWindow);
    decText->setFixedHeight(30);
    decText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    decText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPlainTextEdit *hexText = new QPlainTextEdit(converterWindow);
    hexText->setFixedHeight(30);
    hexText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    hexText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPushButton *btnHexDec = new QPushButton("Hex to Dec", converterWindow);
    QPushButton *btnDecHex = new QPushButton("Dec to Hex", converterWindow);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(converterWindow);
    buttonsLayout->addWidget(btnHexDec);
    buttonsLayout->addWidget(btnDecHex);
    mainLayout->addWidget(labelDec);
    mainLayout->addWidget(decText);
    mainLayout->addWidget(labelHex);
    mainLayout->addWidget(hexText);
    mainLayout->addLayout(buttonsLayout);
    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);

    connect(btnHexDec, &QPushButton::clicked, [hexText, decText]()
    {
        QString hext = hexText->toPlainText().replace("0x", "");
        hext = hext.replace(" ", "");
        if (hext != "") {
            decText->setPlainText(QString::number(hext.toLong(nullptr, 16), 10));
        }
    });

    connect(btnDecHex, &QPushButton::clicked, [hexText, decText]()
    {
        if (decText->toPlainText() != "") {
            hexText->setPlainText(QString::number(decText->toPlainText().toLong(nullptr, 10), 16));
        }
    });

    converterWindow->setCentralWidget(mainWidget);
    converterWindow->show();
}


void Fhex::on_menu_escape_hex_click() {
    QMainWindow *escapeWindow = new QMainWindow(this);
    escapeWindow->setWindowIcon(this->windowIcon());
    escapeWindow->setWindowTitle("Fhex - Escape Hex");
    escapeWindow->setFixedSize(300, 200);
    QVBoxLayout *mainLayout =new QVBoxLayout(escapeWindow);
    QLabel *labelHexString = new QLabel("Hex String:", escapeWindow);
    labelHexString->setFixedHeight(20);
    QLabel *labelEscapedString = new QLabel("Escaped String:", escapeWindow);
    labelEscapedString->setFixedHeight(20);
    QPlainTextEdit *hexStringText = new QPlainTextEdit(escapeWindow);
    hexStringText->setFixedHeight(30);
    hexStringText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    hexStringText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPlainTextEdit *escapedText = new QPlainTextEdit(escapeWindow);
    escapedText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    escapedText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    escapedText->setFixedHeight(30);
    QPushButton *btnEscapeHex = new QPushButton("Escape Hex String", escapeWindow);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(escapeWindow);
    buttonsLayout->addWidget(btnEscapeHex);
    mainLayout->addWidget(labelHexString);
    mainLayout->addWidget(hexStringText);
    mainLayout->addWidget(labelEscapedString);
    mainLayout->addWidget(escapedText);
    mainLayout->addLayout(buttonsLayout);
    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);

    connect(btnEscapeHex, &QPushButton::clicked, [this, hexStringText, escapedText]()
    {
        QString hext = hexStringText->toPlainText().replace(" ", "");
        if (hext != "") {
            if (hext.length() % 2 == 0) {
                QString escapedHex = "";
                for (int i = 0; i < hext.length(); i += 2) {
                    escapedHex += "\\x" + hext[i] + hext[i+1];
                }
                escapedText->setPlainText(escapedHex);
            } else {
                QMessageBox msgBox;
                msgBox.setText("Hex String length is invalid. It is not divisible by 2.");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.setIcon(QMessageBox::Icon::Warning);
                msgBox.setWindowTitle(this->windowTitle());
                msgBox.setWindowIcon(this->windowIcon());
                msgBox.exec();
            }
        }
    });

    escapeWindow->setCentralWidget(mainWidget);
    escapeWindow->show();
}

void Fhex::on_menu_binchart_click() {
    this->binChartView->setVisible(!this->binChartView->isVisible());
}
