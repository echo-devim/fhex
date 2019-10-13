#include "fhex.h"
#include <QDebug>

Fhex::Fhex(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle("Fhex");

    this->hexEditor = new HexEditor();

    /** Menu Initialization **/
    QMenu *file;
    file = menuBar()->addMenu("&File");
    QAction *openFile = new QAction(QIcon::fromTheme("folder-open"), "&Open", this);
    QAction *saveFile = new QAction(QIcon::fromTheme("document-save"), "&Save", this);
    QAction *saveNewFile = new QAction(QIcon::fromTheme("document-save-as"), "&Save as ..", this);
    file->addAction(openFile);
    file->addAction(saveFile);
    file->addAction(saveNewFile);

    connect(openFile, &QAction::triggered, this, &Fhex::on_menu_file_open_click);
    connect(saveFile, &QAction::triggered, this, &Fhex::on_menu_file_save_click);
    connect(saveNewFile, &QAction::triggered, this, &Fhex::on_menu_file_save_as_click);
    /** End Menu Initialization **/

    QGridLayout *gridLayout = new QGridLayout;

    hexTableWidget = new QTableWidget((static_cast<int>(hexEditor->getCurrentData().size()) / this->columns) + 1, this->columns, this);
    charTableWidget = new QTableWidget((static_cast<int>(hexEditor->getCurrentData().size()) / this->columns) + 1, this->columns, this);

    this->setAcceptDrops(true);

    this->hexTableWidget->setMinimumSize(600, 400);
    this->charTableWidget->setMinimumSize(600, 400);

    connect(charTableWidget, &QTableWidget::cellChanged, this, &Fhex::on_cell_changed_charTable);
    connect(hexTableWidget, &QTableWidget::cellChanged, this, &Fhex::on_cell_changed_hexTable);
    connect(hexTableWidget->verticalScrollBar(), &QScrollBar::valueChanged, this, &Fhex::on_scroll_hexTable);
    connect(hexTableWidget, &QTableWidget::cellEntered, this, &Fhex::on_cell_pressed_hexTable);
    connect(hexTableWidget, &QTableWidget::cellClicked, this, &Fhex::on_cell_clicked_hexTable);
    connect(charTableWidget->verticalScrollBar(), &QScrollBar::valueChanged, this, &Fhex::on_scroll_charTable);
    connect(charTableWidget, &QTableWidget::cellEntered, this, &Fhex::on_cell_pressed_charTable);
    connect(charTableWidget, &QTableWidget::cellClicked, this, &Fhex::on_cell_clicked_charTable);

    this->loadTables();
    gridLayout->addWidget(hexTableWidget, 0, 0);
    gridLayout->addWidget(charTableWidget, 0, 1);

    gridLayout->addWidget(&this->statusBar, 2, 0);

    searchBox = new QFrame(this);
    QFormLayout *searchBoxLayout = new QFormLayout(searchBox);
    QHBoxLayout *firstRow = new QHBoxLayout(searchBox);
    QPushButton *searchButton = new QPushButton("Search");
    QPushButton *replaceButton = new QPushButton("Replace");
    connect(searchButton, &QPushButton::clicked, this, &Fhex::on_search_button_click);
    connect(replaceButton, &QPushButton::clicked, this, &Fhex::on_replace_button_click);
    searchButton->setFixedWidth(60);
    replaceButton->setFixedWidth(60);
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
    firstRow->addWidget(searchFormatOption);
    firstRow->addWidget(searchText);
    firstRow->addWidget(searchButton);
    searchBoxLayout->addRow(firstRow);
    searchBoxLayout->addRow(replaceText, replaceButton);
    searchBox->setLayout(searchBoxLayout);
    searchBox->setFixedSize(this->width() - 20, 100);
    searchText->setFixedWidth(searchBox->width() / 2);
    searchBox->setObjectName("container");
    searchBox->setStyleSheet("QFrame#container { border: 1px solid #bbbbbb; padding: 0px; margin: 0px;}");
    searchBox->setVisible(false);

    gridLayout->addWidget(searchBox, 1, 0, 1, 2, Qt::AlignLeft);

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
    int i = index;
    //long len = CHUNK_SIZE;
    long len = hexEditor->getCurrentData().size();


    //Resize the tables in order to contain the data
    this->hexTableWidget->setRowCount((len / this->columns) + 1);
    this->charTableWidget->setRowCount((len / this->columns) + 1);

    for (uint8_t byte : this->hexEditor->getCurrentData()) {
        QTableWidgetItem *hexItem = new QTableWidgetItem(QString("%1").arg(byte, 2, 16, QChar('0')));
        hexTableWidget->setItem(i / 16, i % 16, hexItem);
        QTableWidgetItem *charItem = nullptr;
        if (isprint(byte)){
            charItem = new QTableWidgetItem(QString(byte));
        } else {
            charItem = new QTableWidgetItem(QString(DEFAULT_UNPRINTABLE_CHAR));
        }
        QFont font("Monospace");
        font.setStyleHint(QFont::TypeWriter);
        font.setPixelSize(16);
        charItem->setFont(font);
        charItem->setTextAlignment(Qt::AlignCenter);
        hexItem->setFont(font);
        hexItem->setTextAlignment(Qt::AlignCenter);
        charTableWidget->setItem(i / 16, i % 16, charItem);
        i++;
        if (len <= 0)
            break;
        else
            len--;
    }

    // Fill the last empty cells with empty objects
    for (int j = i; j < (this->charTableWidget->columnCount() * this->charTableWidget->rowCount()); j++) {
        QTableWidgetItem *emptyHexItem = new QTableWidgetItem("");
        QTableWidgetItem *emptyCharItem = new QTableWidgetItem("");
        charTableWidget->setItem(j / 16, j % 16, emptyCharItem);
        hexTableWidget->setItem(j / 16, j % 16, emptyHexItem);
    }

    this->statusBar.setText("Background file load task finished");
    hexTableWidget->resizeColumnsToContents();
    charTableWidget->resizeColumnsToContents();
    this->hexTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->charTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    this->initialized_tables = true;
}

void Fhex::loadTables(long index) {
    this->initialized_tables = false;

    std::thread t(&Fhex::backgroundLoadTables, this, index);

    t.detach();

}

void Fhex::on_scroll_hexTable(int pos) {
    this->charTableWidget->verticalScrollBar()->setValue(pos);
}

void Fhex::on_scroll_charTable(int pos) {
    this->hexTableWidget->verticalScrollBar()->setValue(pos);
}

void Fhex::on_cell_pressed_hexTable(int row, int col) {
    if (this->charTableWidget->item(row, col) != nullptr) {
        this->charTableWidget->item(row, col)->setSelected(true);
        if (!this->doing_selection) {
            this->doing_selection = true;
            this->prev_selected_row = row;
        } else if(row != this->prev_selected_row) {
            this->charTableWidget->clearSelection();
            std::async(&QTableWidget::clearSelection, this->hexTableWidget);
            this->doing_selection = false;
        }
    }
}

void Fhex::on_cell_clicked_hexTable(int row, int col) {
    if (this->charTableWidget->item(row, col) != nullptr) {
        std::async(&QTableWidget::clearSelection, this->charTableWidget);
        this->charTableWidget->item(row, col)->setSelected(true);
        this->doing_selection = false;
        unsigned long offset = static_cast<unsigned long>((this->columns*row) + col);
        this->statusBar.setText("File Offset: 0x" + QString::number(offset, 16) + " (" + QString::number(offset) + ")");
    }
}

void Fhex::on_cell_pressed_charTable(int row, int col) {
    if (this->hexTableWidget->item(row, col) != nullptr) {
        this->hexTableWidget->item(row, col)->setSelected(true);
        if (!this->doing_selection) {
            this->doing_selection = true;
            this->prev_selected_row = row;
        } else if(row != this->prev_selected_row) {
            std::async(&QTableWidget::clearSelection, this->charTableWidget);
            std::async(&QTableWidget::clearSelection, this->hexTableWidget);
            this->doing_selection = false;
        }
    }
}

void Fhex::on_cell_clicked_charTable(int row, int col) {
    if (this->hexTableWidget->item(row, col) != nullptr) {
        std::async(&QTableWidget::clearSelection, this->hexTableWidget);
        this->hexTableWidget->item(row, col)->setSelected(true);
        this->doing_selection = false;
        unsigned long offset = static_cast<unsigned long>((this->columns*row) + col);
        this->statusBar.setText("File Offset: 0x" + QString::number(offset, 16) + " (" + QString::number(offset) + ")");
    }
}

void Fhex::on_cell_changed_charTable(int row, int col) {
    QString orig_text = this->charTableWidget->item(row,col)->text();
    if (orig_text.length() > 0)
        this->charTableWidget->item(row,col)->setText(orig_text.at(0));
    else
        return;
    QChar character(orig_text.at(0));
    if (this->initialized_tables) {
        QBrush b(color_red);
        charTableWidget->item(row,col)->setForeground(b);
        hexTableWidget->item(row,col)->setForeground(b);
        this->hexTableWidget->item(row, col)->setText(QString("%1").arg(character.unicode(), 2, 16, QChar('0')));
    } else if (character.isDigit() || character.isLetter()) {
        QBrush b(color_brown);
        charTableWidget->item(row,col)->setForeground(b);
        hexTableWidget->item(row,col)->setForeground(b);
    }
}

void Fhex::on_cell_changed_hexTable(int row, int col) {
    QString orig_text = this->hexTableWidget->item(row,col)->text();
    if (orig_text.length() > 2)
        this->hexTableWidget->item(row,col)->setText(orig_text.mid(0,2));
    else
        return;
    if (this->initialized_tables) {
        QBrush b(color_red);
        charTableWidget->item(row,col)->setForeground(b);
        hexTableWidget->item(row,col)->setForeground(b);
        QChar ch(orig_text.toUInt(nullptr, 16));
        if (ch.isPrint())
            this->charTableWidget->item(row, col)->setText(ch);
        else
            this->charTableWidget->item(row, col)->setText(DEFAULT_UNPRINTABLE_CHAR);
    }
}

void Fhex::keyPressEvent(QKeyEvent *event) {
    if(event->type() == QKeyEvent::KeyPress) {
        if ((event->key() == Qt::Key_C)  && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            QString selection;
            if (this->hexTableWidget->hasFocus()) {
                for (QTableWidgetItem *item : this->hexTableWidget->selectedItems()) {
                    selection += item->text() + " ";
                }
            } else {
                for (QTableWidgetItem *item : this->charTableWidget->selectedItems()) {
                    //TODO: avoid to copy non-printable characters
                    selection += item->text();
                }
            }
            QApplication::clipboard()->setText(selection);
        } else if ((event->key() == Qt::Key_S)  && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            //vector<uint8_t> newData;
            unsigned long offset = 0;
            for (int i=0; i< hexTableWidget->rowCount(); i++) {
                for (int j=0; j< hexTableWidget->columnCount(); j++) {
                    if ((hexTableWidget->item(i,j) != nullptr) && (hexTableWidget->item(i,j)->foreground().color() == color_red)) {
                        QTableWidgetItem *item = hexTableWidget->item(i,j);
                        this->hexEditor->updateByte(static_cast<uint8_t>(item->text().toULong(nullptr, 16)), offset);
                    }
                    offset++;
                }
            }
        } else if ((event->key() == Qt::Key_V)  && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            if (this->hexTableWidget->hasFocus() && (this->hexTableWidget->selectedItems().first() != nullptr)) {
                QStringList ctext = QApplication::clipboard()->text().split(" ");
                QBrush b(color_red);
                int row = this->hexTableWidget->selectedItems().first()->row();
                int col = this->hexTableWidget->selectedItems().first()->column();
                for (QString &byte : ctext) {
                    this->hexTableWidget->item(row, col)->setText(byte);
                    QChar ch(byte.toUInt(nullptr, 16));
                    if (ch.isPrint())
                        this->charTableWidget->item(row, col)->setText(ch);
                    else
                        this->charTableWidget->item(row, col)->setText(DEFAULT_UNPRINTABLE_CHAR);
                    charTableWidget->item(row, col)->setForeground(b);
                    hexTableWidget->item(row, col)->setForeground(b);
                    if (col > 14) {
                        col = 0;
                        row++;
                    } else {
                        col++;
                    }
                }
            } else if (this->charTableWidget->hasFocus() && (this->charTableWidget->selectedItems().first() != nullptr)) {
                QString ctext = QApplication::clipboard()->text();
                QBrush b(color_red);
                int row = this->charTableWidget->selectedItems().first()->row();
                int col = this->charTableWidget->selectedItems().first()->column();
                for (QChar &ch : ctext) {
                    this->charTableWidget->item(row, col)->setText(ch);
                    this->hexTableWidget->item(row, col)->setText(QString("%1").arg(ch.unicode(), 2, 16, QChar('0')));
                    charTableWidget->item(row, col)->setForeground(b);
                    hexTableWidget->item(row, col)->setForeground(b);
                    if (col >= (this->columns - 1)) {
                        col = 0;
                        row++;
                    } else {
                        col++;
                    }
                }
            }
        } else if ((event->key() == Qt::Key_F)  && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            this->searchBox->setVisible(!this->searchBox->isVisible());
            clearBackgroundColor(this->charTableWidget);
            clearBackgroundColor(this->hexTableWidget);
        } else if ((event->key() == Qt::Key_Z)  && QApplication::keyboardModifiers() && Qt::ControlModifier) {
            //TODO
        }
    }
}

bool Fhex::loadFile(QString path) {
    this->statusBar.setText("Loading " + path);
    auto t1 = std::chrono::high_resolution_clock::now();
    bool res = this->hexEditor->loadFileAsync(path.toStdString());
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    this->statusBar.setText("File loaded (" + QString::number(this->hexEditor->getCurrentData().size() / 1024) + " KB) in " + QString::number(duration / 1000.) + "s");
    return res;
}

void Fhex::on_menu_file_open_click() {
    QString path(this->hexEditor->getCurrentPath().c_str());
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), path,
        tr("All Files (*)"));
    if (fileName != "") {
        this->loadFile(fileName);
        this->loadTables(); //update ui
    }
}

void Fhex::on_menu_file_save_click() {
    unsigned long offset = 0;
    for (int i=0; i< hexTableWidget->rowCount(); i++) {
        for (int j=0; j< hexTableWidget->columnCount(); j++) {
            if ((hexTableWidget->item(i,j) != nullptr) && (hexTableWidget->item(i,j)->foreground().color() == color_red)) {
                QTableWidgetItem *item = hexTableWidget->item(i,j);
                this->hexEditor->updateByte(static_cast<uint8_t>(item->text().toULong(nullptr, 16)), offset);
            }
            offset++;
        }
    }
}

QString Fhex::getTextFromCells(QTableWidget *table, int row, int col, int len) {
    QString result;
    for (int i = 0; (i < len) && (table->item(row, col) != nullptr); i++) {
        result += table->item(row, col)->text();
        if (col >= (this->columns - 1)) {
            col = 0;
            row++;
        } else {
            col++;
        }
    }
    return result;
}

void Fhex::highlightCells(QTableWidget *table, int row, int col, int len, QColor color) {
    this->initialized_tables = false;
    for (int i = 0; (i < len) && (table->item(row, col) != nullptr); i++) {
        table->item(row, col)->setBackground(QBrush(color));
        if (col >= (this->columns - 1)) {
            col = 0;
            row++;
        } else {
            col++;
        }
    }
    this->initialized_tables = true;
}

void Fhex::on_search_button_click() {
    int matches = 0;
    if (this->searchFormatOption->currentText() == "UTF-8") {
        for (int i=0; i< charTableWidget->rowCount(); i++) {
            for (int j=0; j< charTableWidget->columnCount(); j++) {
                if ((charTableWidget->item(i,j) != nullptr)
                        && (getTextFromCells(charTableWidget, i, j, this->searchText->toPlainText().length()) == this->searchText->toPlainText())) {
                    matches++;
                    highlightCells(charTableWidget, i, j, this->searchText->toPlainText().length(), color_yellow);
                    highlightCells(hexTableWidget, i, j, this->searchText->toPlainText().length(), color_gray);
                }
            }
        }
    } else if (this->searchFormatOption->currentText() == "HEX") {
        QString searchedText = this->searchText->toPlainText().toLower().replace(" ", "");
        for (int i=0; i< hexTableWidget->rowCount(); i++) {
            for (int j=0; j< hexTableWidget->columnCount(); j++) {
                if ((hexTableWidget->item(i,j) != nullptr)
                        && (getTextFromCells(hexTableWidget, i, j, searchedText.length()/2) == searchedText)) {
                    matches++;
                    highlightCells(hexTableWidget, i, j, searchedText.length()/2, color_yellow);
                    highlightCells(charTableWidget, i, j, searchedText.length()/2, color_gray);
                }
            }
        }
    }

    this->statusBar.setText("Found " + QString::number(matches) + " matches");
}

void Fhex::on_replace_button_click() {
    int occurences = 0;
    if (this->searchFormatOption->currentText() == "UTF-8") {
        for (int i=0; i< charTableWidget->rowCount(); i++) {
            for (int j=0; j< charTableWidget->columnCount(); j++) {
                if ((charTableWidget->item(i,j) != nullptr)
                        && (getTextFromCells(charTableWidget, i, j, this->searchText->toPlainText().length()) == this->searchText->toPlainText())) {
                    occurences++;
                    replaceTextInCells(charTableWidget, i, j, this->replaceText->toPlainText());
                }
            }
        }
    } else if (this->searchFormatOption->currentText() == "HEX") {
        QString searchedText = this->searchText->toPlainText().toLower().replace(" ", "");
        for (int i=0; i< hexTableWidget->rowCount(); i++) {
            for (int j=0; j< hexTableWidget->columnCount(); j++) {
                if ((hexTableWidget->item(i,j) != nullptr)
                        && (getTextFromCells(hexTableWidget, i, j, searchedText.length()/2) == searchedText)) {
                    occurences++;
                    replaceByteInCells(hexTableWidget, i, j, this->replaceText->toPlainText());
                }
            }
        }
    }

    this->statusBar.setText("Replaced " + QString::number(occurences) + " occurrences");
}

void Fhex::replaceTextInCells(QTableWidget *table, int row, int col, QString text) {
    for (int i = 0; (i < text.length()) && (table->item(row, col) != nullptr); i++) {
        table->item(row, col)->setText(text.at(i));
        if (col >= (this->columns - 1)) {
            col = 0;
            row++;
        } else {
            col++;
        }
    }
}

void Fhex::replaceByteInCells(QTableWidget *table, int row, int col, QString text) {
    text = text.toLower().replace(" ", "");
    for (int i = 0; (i < text.length()) && (table->item(row, col) != nullptr); i+=2) {
        table->item(row, col)->setText(text.mid(i, 2));
        if (col >= (this->columns - 1)) {
            col = 0;
            row++;
        } else {
            col++;
        }
    }
}


void Fhex::clearBackgroundColor(QTableWidget *table) {
    this->initialized_tables = false;
    for (int i=0; i< charTableWidget->rowCount(); i++) {
        for (int j=0; j< charTableWidget->columnCount(); j++) {
            if (table->item(i, j) != nullptr)
                table->item(i, j)->setBackground(QBrush(color_white));
        }
    }
    this->initialized_tables = true;
}

void Fhex::on_menu_file_save_as_click() {
    QString path(this->hexEditor->getCurrentPath().c_str());
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save File"), path,
        tr("All Files (*)"));
    if (fileName != "") {
        vector<uint8_t> newData;
        for (int i=0; i< hexTableWidget->rowCount(); i++) {
            for (int j=0; j< hexTableWidget->columnCount(); j++) {
                QTableWidgetItem *item = hexTableWidget->item(i,j);
                if (item != nullptr)
                    newData.push_back(static_cast<uint8_t>(item->text().toULong(nullptr, 16)));
            }
        }
        this->hexEditor->setCurrentData(newData);
        this->hexEditor->saveDataToFile(fileName.toStdString());
    }
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
