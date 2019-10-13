#ifndef FHEX_H
#define FHEX_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QGridLayout>
#include <QObject>
#include <QBrush>
#include <QByteArray>
#include <QFont>
#include <QScrollBar>
#include <QColor>
#include <QClipboard>
#include <QApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QFrame>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDropEvent>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QThread>
#include <iostream>
#include <chrono>
#include <thread>
#include <future>

#include "core/hexeditor.h"

#define DEFAULT_UNPRINTABLE_CHAR "."
#define CHUNK_SIZE 1024

const static QColor color_brown(128, 56, 15);
const static QColor color_red(250, 5, 5);
const static QColor color_yellow(255, 226, 148);
const static QColor color_white(255, 255, 255);
const static QColor color_gray(173, 172, 168);

class Fhex : public QMainWindow
{
    Q_OBJECT

public:
    Fhex(QWidget *parent = nullptr);
    ~Fhex();

private:
    QFrame *searchBox;
    QComboBox *searchFormatOption;
    QPlainTextEdit *searchText;
    QPlainTextEdit *replaceText;
    QTableWidget *hexTableWidget;
    QTableWidget *charTableWidget;
    HexEditor *hexEditor;
    QLabel statusBar;
    int columns = 16;
    //vector<QString> history_changed_bytes;
    long prev_selected_row = -1;
    bool doing_selection = false;
    bool initialized_tables = false;
    void backgroundLoadTables(long index);
    QString getTextFromCells(QTableWidget *table, int row, int col, int len);
    void highlightCells(QTableWidget *table, int row, int col, int len, QColor color);
    void replaceTextInCells(QTableWidget *table, int row, int col, QString text);
    void replaceByteInCells(QTableWidget *table, int row, int col, QString text);
    void clearBackgroundColor(QTableWidget *table);
    void loadTables(long index = 0);
    void keyPressEvent( QKeyEvent *event );
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *e);
    bool loadFile(QString path);

public slots:
    void on_cell_changed_charTable(int row, int col);
    void on_cell_changed_hexTable(int row, int col);
    void on_scroll_hexTable(int pos);
    void on_scroll_charTable(int pos);
    void on_cell_pressed_hexTable(int row, int col);
    void on_cell_clicked_hexTable(int row, int col);
    void on_cell_pressed_charTable(int row, int col);
    void on_cell_clicked_charTable(int row, int col);
    void on_menu_file_open_click();
    void on_menu_file_save_click();
    void on_menu_file_save_as_click();
    void on_search_button_click();
    void on_replace_button_click();
};

#endif // FHEX_H
