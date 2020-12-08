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
#include <QListWidget>
#include <QListWidgetItem>
#include <QtCharts>
#include <QLineSeries>
#include <QMessageBox>
#include <iostream>
#include <chrono>
#include <thread>
#include <future>

#include "qhexedit.h"
#include "core/hexeditor.h"
#include "fasm.h"

#define MAX_DIFF_BYTES 3000
#define DEFAULT_UNPRINTABLE_CHAR "."
#define CHUNK_SIZE 1024
#define CHART_DENSITY 1000 //A lower value implies a greater density
#define CHART_LINE_COLOR 0xab621f
#define CHART_BACKGROUND_COLOR 0x1c140a

#define DIFF_STYLE "QLabel { background-color: rgba(200, 100, 100, 70); }"

using namespace QtCharts;

class Fhex : public QMainWindow
{
    Q_OBJECT

public:
    Fhex(QWidget *parent = nullptr, QApplication *app = nullptr, QString filepath = "");
    ~Fhex();

private:
    Fasm *fasm;
    qint64 lastCursorPos = 0;
    qint64 currentCursorPos = 0;
    QChartView *binChartView;
    QApplication *app;
    QProgressBar *progressBar;
    QCheckBox regexCheckBox;
    QFrame *searchBox;
    QComboBox *searchFormatOption;
    QPlainTextEdit *searchText;
    QPushButton *searchButton;
    QPlainTextEdit *replaceText;
    QPushButton *replaceButton;
    QTableWidget *hexTableWidget;
    QTableWidget *charTableWidget;
    QListWidget *listOffsets;
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
    void compare(QString filename);
    void addFloatingLabel(qint64 offset, int len, QString text, QString style = "", bool addComment = false);
    void clearFloatingLabels();
    void findPatterns();
    void saveDataToFile(string path);
    void loadBinChart();
    void updateOffsetBar();
    void updateOffsetBarWithSelection();

public slots:
    void on_editor_mouse_click();
    void on_editor_mouse_move();
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
    void on_menu_find_patterns_click();
    void on_list_offset_item_click(QListWidgetItem *item);
    void on_menu_offset_list_click();
    void on_menu_new_file_click();
    bool eventFilter(QObject* o, QEvent* e);
    void on_menu_hex_dec_converter_click();
    void on_menu_escape_hex_click();
    void on_binchart_click(const QPointF &p);
    void on_menu_binchart_click();
    void on_menu_fasm_click();
    void on_menu_about_click();

};

#endif // FHEX_H
