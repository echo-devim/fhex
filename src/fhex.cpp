#include "fhex.h"
#include <QDebug>

Fhex::Fhex(QWidget *parent, QApplication *app)
    : QMainWindow(parent)
{
    this->app = app;
    this->setWindowTitle("Fhex");
    //this->setStyleSheet("QMainWindow { background-color: #010301 }");
    this->setMinimumSize(800, 500);

    this->hexEditor = new HexEditor();

    /** Menu Initialization **/
    QMenu *file;
    file = menuBar()->addMenu("&File");
    QAction *openFile = new QAction(QIcon::fromTheme("folder-open"), "&Open", this);
    QAction *saveFile = new QAction(QIcon::fromTheme("document-save"), "&Save", this);
    QAction *saveNewFile = new QAction(QIcon::fromTheme("document-save-as"), "&Save as ..", this);
    QAction *openNewWindow = new QAction(QIcon::fromTheme("window-new"), "&New Window", this);
    file->addAction(openFile);
    file->addAction(saveFile);
    file->addAction(saveNewFile);
    file->addAction(openNewWindow);
    QMenu *edit;
    edit = menuBar()->addMenu("&Edit");
    QAction *find = new QAction(QIcon::fromTheme("edit-find"), "&Find", this);
    edit->addAction(find);
    QAction *convert = new QAction(QIcon::fromTheme("convert"), "&Convert Bytes", this);
    edit->addAction(convert);

    connect(openFile, &QAction::triggered, this, &Fhex::on_menu_file_open_click);
    connect(saveFile, &QAction::triggered, this, &Fhex::on_menu_file_save_click);
    connect(saveNewFile, &QAction::triggered, this, &Fhex::on_menu_file_save_as_click);
    connect(openNewWindow, &QAction::triggered, this, &Fhex::on_menu_file_new_window_click);
    connect(find, &QAction::triggered, this, &Fhex::on_menu_find_click);
    connect(find, &QAction::triggered, this, &Fhex::on_menu_convert_bytes_click);
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

    connect(qhex, &QHexEdit::handle_keyPressEvent, this, &Fhex::keyPressEvent);
    connect(this->qhex, &QHexEdit::handle_mouseClick, this, &Fhex::on_editor_mouse_click);

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
    QPushButton *searchButton = new QPushButton("Search");
    QPushButton *replaceButton = new QPushButton("Replace");
    QPushButton *replaceAllButton = new QPushButton("Replace All");
    connect(searchButton, &QPushButton::clicked, this, &Fhex::on_search_button_click);
    connect(replaceButton, &QPushButton::clicked, this, &Fhex::on_replace_button_click);
    connect(replaceAllButton, &QPushButton::clicked, this, &Fhex::on_replace_all_button_click);
    searchButton->setFixedWidth(60);
    replaceButton->setFixedWidth(60);
    replaceAllButton->setFixedWidth(80);
    this->searchText = new QPlainTextEdit(searchBox);
    this->replaceText = new QPlainTextEdit(searchBox);
    searchText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    searchText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    replaceText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    replaceText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->searchFormatOption = new QComboBox(this);
    searchFormatOption->addItem("UTF-8");
    //TODO: formatOption->addItem("UTF-16");
    searchFormatOption->addItem("HEX");
    searchFormatOption->setFixedWidth(80);
    replaceText->setStyleSheet("QPlainTextEdit { margin-left: 90px; }");
    firstRow->addWidget(searchFormatOption);
    firstRow->addWidget(searchText);
    this->regexCheckBox.setText("regex");
    this->regexCheckBox.setChecked(true);
    this->regexCheckBox.setFixedWidth(60);
    firstRow->addWidget(&regexCheckBox);
    firstRow->addWidget(searchButton);
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

    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(gridLayout);

    this->statusBar.setText("Fhex loaded");
    this->setCentralWidget(mainWidget);

}

Fhex::~Fhex()
{
    delete this->hexEditor;
}

void Fhex::backgroundLoadTables(long index) {
    this->qhex->setData(QByteArray(reinterpret_cast<const char*>(this->hexEditor->getCurrentData().data()), this->hexEditor->fileSize));
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
        if ((event->key() == Qt::Key_S)  && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            this->on_menu_file_save_click();
        } else if ((event->key() == Qt::Key_F)  && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            this->on_menu_find_click();
        } else if ((event->key() == Qt::Key_B)  && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            this->on_menu_convert_bytes_click();
        } else if ((event->key() == Qt::Key_N)  && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            this->on_menu_file_new_window_click();
        }
    }
}

void Fhex::on_menu_find_click() {
    this->searchBox->setVisible(!this->searchBox->isVisible());
}

bool Fhex::loadFile(QString path) {
    this->progressBar->setVisible(true);
    this->progressBar->setValue(0);
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

void Fhex::on_menu_file_save_click() {
    this->statusBar.setText("Saving file..");
    unsigned long changes = 0;
    if (this->qhex->isModified()) {
        if (this->qhex->data().size() == this->hexEditor->fileSize) {
            unsigned long offset = 0;
            for (uint8_t byte : this->hexEditor->getCurrentData()) {
                QByteArray qbarr = this->qhex->dataAt(static_cast<long long>(offset), 1);
                vector<uint8_t> newData(qbarr.begin(), qbarr.end());
                uint8_t currByte = newData.at(0);
                if (byte != currByte) {
                    this->hexEditor->updateByte(currByte, offset);
                    changes++;
                }
                offset++;
            }
            this->statusBar.setText("File updated! " + QString::number(changes) + " byte(s) changed.");
        } else {
            on_menu_file_save_as_click();
        }
    } else {
        this->statusBar.setText("No changes were made");
    }
}

void Fhex::on_search_button_click() {
    qint64 start = this->qhex->cursorPosition() + 1;
    if (this->searchFormatOption->currentText() == "UTF-8") {
        qint64 res = this->qhex->indexOf(this->searchText->toPlainText().toUtf8(), start, this->regexCheckBox.isChecked());
        if (res < 0) {
            this->statusBar.setText("No match found");
        } else {
            this->statusBar.setText("Found match at 0x" + QString::number(res, 16));
            this->qhex->setCursorPosition(res);
        }
    } else if (this->searchFormatOption->currentText() == "HEX") {
        QString searchedText = this->searchText->toPlainText().toLower().replace(" ", "");
        qint64 res = this->qhex->indexOf(QByteArray::fromHex(searchedText.toLatin1()), start, this->regexCheckBox.isChecked());
        if (res < 0) {
            this->statusBar.setText("No match found");
        } else {
            this->statusBar.setText("Found match at 0x" + QString::number(res, 16));
            this->qhex->setCursorPosition(res);
        }
    }
}

qint64 Fhex::replaceBytes(QString searchText, QString replaceText, bool isHex) {
    qint64 start = this->qhex->cursorPosition();
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
        this->hexEditor->getCurrentData().clear();
        this->hexEditor->getCurrentData().shrink_to_fit();
        this->hexEditor->getCurrentData().insert(this->hexEditor->getCurrentData().end(), this->qhex->data().begin(), this->qhex->data().end());
        this->hexEditor->saveDataToFile(fileName.toStdString());
        this->statusBar.setText("File saved as " + fileName);
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
    const QUrl url = event->mimeData()->urls().first();
    QString fileName = url.toLocalFile();
    this->loadFile(fileName);
    this->loadTables();
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
