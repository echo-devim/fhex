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
#include <QVBoxLayout>
#include <QProgressBar>
#include <QCheckBox>
#include <QInputDialog>
#include <QTextEdit>
#include <iostream>
#include <chrono>
#include <thread>
#include <future>

#include "qhexedit.h"
#include "core/hexeditor.h"

#define DEFAULT_UNPRINTABLE_CHAR "."
#define CHUNK_SIZE 1024

#define DIFF_STYLE "QLabel { background-color: rgba(200, 100, 100, 70); }"

class Fhex : public QMainWindow
{
    Q_OBJECT

public:
    Fhex(QWidget *parent = nullptr, QApplication *app = nullptr);
    ~Fhex();

private:
    QApplication *app;
    QProgressBar *progressBar;
    QCheckBox regexCheckBox;
    QFrame *searchBox;
    QComboBox *searchFormatOption;
    QPlainTextEdit *searchText;
    QPlainTextEdit *replaceText;
    QTableWidget *hexTableWidget;
    QTableWidget *charTableWidget;
    HexEditor *hexEditor;
    QHexEdit *qhex;
    QFrame *convertBox;
    QLabel statusBar;
    QLabel offsetBar;
    QLabel convertLabel;
    vector<QLabel*> floatingLabels;
    int prev_vscrollbar_value;
    int prev_hscrollbar_value;
    bool initialized_tables = false;
    qint64 replaceBytes(QString searchText, QString replaceText, bool isHex = true);
    void backgroundLoadTables(long index);
    void clearBackgroundColor(QTableWidget *table);
    void loadTables(long index = 0);
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *e);
    bool loadFile(QString path);
    void keyPressEvent( QKeyEvent *event );
    void compare();
    void addFloatingLabel(qint64 offset, int len, QString text, QString style = "");

public slots:
    void on_editor_mouse_click();
    void on_menu_file_open_click();
    void on_menu_file_diff_click();
    void on_menu_file_save_click();
    void on_menu_file_save_as_click();
    void on_search_button_click();
    void on_replace_button_click();
    void on_convert_button_click();
    void on_replace_all_button_click();
    void on_menu_find_click();
    void on_menu_convert_bytes_click();
    void on_menu_file_new_window_click();
    void on_back_search_button_click();
    void on_menu_goto_offset_click();
    void on_menu_open_text_viewer_click();
    void on_vertical_scrollbar_change(int value);
    void on_horizontal_scrollbar_change(int value);
};

#endif // FHEX_H
