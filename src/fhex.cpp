#include "fhex.h"
#include <QDebug>

Fhex::Fhex(QWidget *parent, QApplication *app)
    : QMainWindow(parent)
{
    this->app = app;
    this->setWindowTitle("Fhex");
    this->setWindowIcon(QIcon("/usr/share/icons/fhex.png"));
    this->setMinimumSize(800, 500);

    this->prev_vscrollbar_value = 0;
    this->prev_hscrollbar_value = 0;
    this->hexEditor = new HexEditor();

    /** Menu Initialization **/
    QMenu *file;
    file = menuBar()->addMenu("&File");
    QAction *newFile = new QAction(QIcon::fromTheme("document-new"), "&New", this);
    QAction *openFile = new QAction(QIcon::fromTheme("folder-open"), "&Open", this);
    QAction *diffFile = new QAction(QIcon::fromTheme("folder-open"), "&Diff..", this);
    QAction *saveFile = new QAction(QIcon::fromTheme("document-save"), "&Save", this);
    saveFile->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
    QAction *saveNewFile = new QAction(QIcon::fromTheme("document-save-as"), "&Save as ..", this);
    QAction *openNewWindow = new QAction(QIcon::fromTheme("window-new"), "&New Window", this);
    openNewWindow->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_N));
    file->addAction(newFile);
    file->addAction(openFile);
    file->addAction(diffFile);
    file->addAction(saveFile);
    file->addAction(saveNewFile);
    file->addAction(openNewWindow);
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
    QAction *findPatternsMenu = new QAction(QIcon::fromTheme("find"), "&Find Patterns", this);
    findPatternsMenu->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_P));
    edit->addAction(findPatternsMenu);
    QAction *menuOffsetList = new QAction(QIcon::fromTheme("find"), "&Show/Hide Offset List", this);
    menuOffsetList->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
    edit->addAction(menuOffsetList);

    connect(newFile, &QAction::triggered, this, &Fhex::on_menu_new_file_click);
    connect(menuOffsetList, &QAction::triggered, this, &Fhex::on_menu_offset_list_click);
    connect(findPatternsMenu, &QAction::triggered, this, &Fhex::on_menu_find_patterns_click);
    connect(openTextViewer, &QAction::triggered, this, &Fhex::on_menu_open_text_viewer_click);
    connect(gotoOffset, &QAction::triggered, this, &Fhex::on_menu_goto_offset_click);
    connect(diffFile, &QAction::triggered, this, &Fhex::on_menu_file_diff_click);
    connect(openFile, &QAction::triggered, this, &Fhex::on_menu_file_open_click);
    connect(saveFile, &QAction::triggered, this, &Fhex::on_menu_file_save_click);
    connect(saveNewFile, &QAction::triggered, this, &Fhex::on_menu_file_save_as_click);
    connect(openNewWindow, &QAction::triggered, this, &Fhex::on_menu_file_new_window_click);
    connect(find, &QAction::triggered, this, &Fhex::on_menu_find_click);
    connect(convert, &QAction::triggered, this, &Fhex::on_menu_convert_bytes_click);

    QMenu *tools;
    tools = menuBar()->addMenu("&Tools");
    QAction *hexDec = new QAction(QIcon::fromTheme("edit-find"), "&Hex<->Dec", this);
    hexDec->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_H));
    tools->addAction(hexDec);

    connect(hexDec, &QAction::triggered, this, &Fhex::on_menu_hex_dec_converter_click);

    /** End Menu Initialization **/

    QGridLayout *gridLayout = new QGridLayout;

    this->setAcceptDrops(true);

    qhex = new QHexEdit(this);
    qhex->setMinimumWidth(600);
    qhex->setStyleSheet("QHexEdit { background-color: #17120f; color: #ebe5e1; }");
    qhex->setAddressAreaColor(color_dark_gray);
    qhex->setSelectionColor(color_dark_yellow);
    qhex->setHighlightingColor(color_dark_violet);

    gridLayout->addWidget(qhex, 0, 0, 1, 2);

    connect(this->qhex, &QHexEdit::handle_keyPressEvent, this, &Fhex::keyPressEvent);
    connect(this->qhex, &QHexEdit::handle_mouseClick, this, &Fhex::on_editor_mouse_click);
    connect(this->qhex->verticalScrollBar(), &QScrollBar::valueChanged, this, &Fhex::on_vertical_scrollbar_change);
    connect(this->qhex->horizontalScrollBar(), &QScrollBar::valueChanged, this, &Fhex::on_horizontal_scrollbar_change);

    this->progressBar = new QProgressBar(this);
    this->progressBar->setRange(0, 100);
    this->progressBar->setTextVisible(false);
    this->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #c22a19; border: 1px solid #912114;}");
    this->progressBar->setFixedHeight(3);
    this->progressBar->setVisible(false);
    gridLayout->addWidget(progressBar, 2, 0, 1, 2);

    gridLayout->addWidget(&this->statusBar, 3, 0);
    gridLayout->addWidget(&this->offsetBar, 3, 1, Qt::AlignRight);

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

    gridLayout->addWidget(searchBox, 1, 0, 1, 2, Qt::AlignLeft);

    convertBox = new QFrame(this);
    QVBoxLayout *convertBoxLayout = new QVBoxLayout(convertBox);
    QPushButton *convertButton = new QPushButton("Convert Selection");
    convertBoxLayout->addWidget(convertButton);
    this->convertLabel.setTextInteractionFlags(Qt::TextSelectableByMouse);
    convertBoxLayout->addWidget(&this->convertLabel);
    convertBox->setLayout(convertBoxLayout);
    connect(convertButton, &QPushButton::clicked, this, &Fhex::on_convert_button_click);
    convertBox->setVisible(false);

    gridLayout->addWidget(convertBox, 0, 3);

    listOffsets = new QListWidget(this);
    connect(listOffsets, &QListWidget::itemClicked, this, &Fhex::on_list_offset_item_click);
    listOffsets->setFixedWidth(200);
    listOffsets->setStyleSheet("QListWidget { background-color: #0a0600; color: #e3e3e3; font-size: 14px;};");
    listOffsets->setVisible(false);
    gridLayout->addWidget(listOffsets, 0, 3);

    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(gridLayout);

    this->statusBar.setText("Fhex loaded");
    this->setCentralWidget(mainWidget);

}

Fhex::~Fhex()
{
    delete this->hexEditor;
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
        if (offset <= static_cast<long long>(this->hexEditor->fileSize)) {
            this->qhex->setCursorPosition(offset * 2);
            this->qhex->ensureVisible();
        } else {
            this->statusBar.setText("Error: Out-of-bound offset specified");
        }
    }
}

void Fhex::backgroundLoadTables(long index) {
    this->qhex->setData(reinterpret_cast<const char*>(this->hexEditor->getCurrentData().data()), this->hexEditor->fileSize);

    this->initialized_tables = true;
}

void Fhex::loadTables(long index) {
    this->initialized_tables = false;

    std::thread t(&Fhex::backgroundLoadTables, this, index);
    t.detach();

}

void Fhex::on_editor_mouse_click() {
    qint64 offset = this->qhex->cursorPosition() / 2;
    this->offsetBar.setText("File Offset: 0x" + QString::number(offset, 16) + " (" + QString::number(offset) + ") | "
                            + "File Size: " + QString::number(this->hexEditor->fileSize / 1024) + " KB");
}

void Fhex::keyPressEvent(QKeyEvent *event) {
    if(event->type() == QKeyEvent::KeyPress) {
        if ((event->key() == Qt::Key_Space) && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            pair<qint64,qint64> offsets = this->qhex->selectedOffsets();
            QApplication::clipboard()->setText(QString::fromWCharArray(this->hexEditor->getCurrentDataAsWString(offsets.first, offsets.second - offsets.first).c_str()));
        } else if (event->key() == Qt::Key_F5) {
            this->loadFile(this->hexEditor->getCurrentPath().c_str());
            this->loadTables();
        }
    }
}

void Fhex::on_menu_find_patterns_click() {
    findPatterns();
}

void Fhex::findPatterns() {
    clearFloatingLabels();
    this->statusBar.setText("Searching patterns..");
    this->statusBar.repaint();
    long patterns = 0;
    vector<Match *> matches = this->hexEditor->findPatterns();
    for (Match *m : matches) {
        // render highlight area
        QString style("QLabel { color: #fbfbfb; padding: 2px; background-color: ");
        style += m->color.c_str();
        style += " };";
        addFloatingLabel(m->index, m->length, m->message.c_str(), style, true);
        delete m;
        patterns++;
    }
    this->statusBar.setText("Found " + QString::number(patterns) + " patterns");
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

bool Fhex::loadFile(QString path) {
    this->qhex->clear();
    this->progressBar->setVisible(true);
    this->progressBar->setValue(0);
    this->clearFloatingLabels();
    this->statusBar.setText("Loading " + path);
    auto t1 = std::chrono::high_resolution_clock::now();
    bool res = this->hexEditor->loadFileAsync(path.toStdString());
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();

    while(!this->hexEditor->isFileLoaded()) {
        int val = static_cast<int>(this->hexEditor->bytesRead * 100 / this->hexEditor->fileSize);
        this->progressBar->setValue(val);
        this->statusBar.setText("Loading " + QString::number(val) + "%");
        this->repaint();
        this->app->processEvents();
        std::this_thread::sleep_for(100ms);
    }
    this->progressBar->setVisible(false);

    this->statusBar.setText("File loaded (" + QString::number(this->hexEditor->fileSize / 1024) + " KB) in " + QString::number(duration / 1000.) + "s");
    return res;
}

void Fhex::on_menu_file_open_click() {
    QString path(this->hexEditor->getCurrentPath().c_str());
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), path,
        tr("All Files (*)"));
    if (fileName != "") {
        this->loadFile(fileName);
        this->loadTables();
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

    this->hexEditor->bytesRead = 0;
    this->progressBar->setVisible(true);
    this->progressBar->setValue(0);
    this->statusBar.setText("Comparing file.. please wait");

    future<vector<pair<unsigned long, uint8_t>>> fut_res = async([this, filename]()
    {
        HexEditor newHexEditor;
        newHexEditor.loadFileAsync(filename.toStdString());
        while(!newHexEditor.isFileLoaded()) {
            std::this_thread::sleep_for(100ms);
        }
        return this->hexEditor->compareTo(newHexEditor);
    });

    while (fut_res.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready) {
        int val = (this->hexEditor->bytesRead * 100) / this->hexEditor->fileSize;
        this->progressBar->setValue(val);
        this->repaint();
        this->app->processEvents();
    }

    this->progressBar->setVisible(false);

    vector<pair<unsigned long, uint8_t>> res = fut_res.get();
    unsigned long changes = res.size();
    unsigned long start_offset = 0;
    unsigned long offset = 0;
    QByteArray diff_bytes;
    for (pair<unsigned long, uint8_t> p : res) {
        if (offset == 0) {
            start_offset = p.first;
            offset = start_offset;
            listOffsets->addItem("0x" + QString::number(start_offset, 16));
        }
        if (p.first - offset > 0) {
            addFloatingLabel(start_offset, static_cast<int>(diff_bytes.size()), "Compared file:\r\n" + diff_bytes.toHex(' ') + "\r\n-----------\r\n" + diff_bytes, DIFF_STYLE);
            diff_bytes.clear();
            offset = 0;
        } else {
            offset++;
        }
        diff_bytes.push_back(static_cast<char>(p.second));
    }
    if (diff_bytes.size() > 0) {
        addFloatingLabel(start_offset, static_cast<int>(diff_bytes.size()), "After:\r\n" + diff_bytes.toHex(' ') + "\r\n-----------\r\n" + diff_bytes, DIFF_STYLE);
        diff_bytes.clear();
    }

    if (changes == 0)
        this->statusBar.setText("Files are equal");
    else {
        this->statusBar.setText("Found " + QString::number(changes) + " different bytes");
        listOffsets->setVisible(true);
    }
}

void Fhex::on_menu_file_save_click() {
    this->statusBar.setText("Saving file..");
    if (this->qhex->isModified()) {
        if (this->qhex->data().size() == this->hexEditor->fileSize) {
            saveDataToFile(this->hexEditor->getCurrentPath());
            this->statusBar.setText("File updated!");
        } else {
            on_menu_file_save_as_click();
        }
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
    long matches = 0;
    qint64 res = 1;
    bool isHex = (this->searchFormatOption->currentText() == "HEX");
    while (res >= 0) {
        res = replaceBytes(this->searchText->toPlainText(), this->replaceText->toPlainText(), isHex);
        if (res >= 0) {
            matches++;
            this->qhex->setCursorPosition(res + 1);
        }
    }

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
    this->hexEditor->fileSize = this->qhex->data().size();
    this->hexEditor->saveDataToFile(path);
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
    const QUrl url = event->mimeData()->urls().at(0);
    QString fileName = url.toLocalFile();
    this->loadFile(fileName);
    this->loadTables();
    //More than one file, compare them
    if (event->mimeData()->urls().size() > 1) {
        this->compare(event->mimeData()->urls().at(1).toLocalFile());
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

void Fhex::on_menu_goto_offset_click() {
    bool ok;
    QString text = QInputDialog::getText(this, "Goto Offset", "Offset:", QLineEdit::Normal, "", &ok);
    text.replace(" ", "");
    text.replace("0x", "");
    if (ok && !text.isEmpty()) {
        qint64 offset = text.toLongLong(nullptr, 16);
        if (offset <= static_cast<long long>(this->hexEditor->fileSize)) {
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
        commentLabel->move(p.x() + (this->width() / 2), p.y());
        commentLabel->adjustSize();
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
    //Add one null byte
    this->qhex->setData(QByteArray::fromStdString("."));
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
    QPlainTextEdit *hexText = new QPlainTextEdit(converterWindow);
    hexText->setFixedHeight(30);
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
