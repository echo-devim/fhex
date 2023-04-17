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
#include <QFileInfo>
#include <iostream>
#include <chrono>
#include <thread>
#include <future>

#include "core/json.h"

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

using json = nlohmann::json;
using namespace QtCharts;

class Fhex : public QMainWindow
{
    Q_OBJECT

public:
    Fhex(QWidget *parent = nullptr, QApplication *app = nullptr, QString filepath = "");
    ~Fhex();

private:
    // RELEASE VERSION
    string version = "3.0.3";

    // JSON
    json jconfig;
    json jsettings;

    // FILES
    string settingsFile;
    string settingsPath;
    string patternsFile;

    // PATTERNS
    bool patternsEnabled;

    Fasm *fasm;
    unsigned long file_size_limit = 1073741824; //1GB is the maximum amount of bytes that the hex editor can load into memory (configurable)
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
    QPushButton *searchAllButton;
    QPlainTextEdit *replaceText;
    QPushButton *replaceButton;
    QTableWidget *hexTableWidget;
    QTableWidget *charTableWidget;
    QListWidget *listOffsets;
    HexEditor *hexEditor;
    QHexEdit *qhex;
    QPushButton *backSearchButton;
    short fontSize;
    QString fontName;
    QFrame *convertBox;
    QLabel statusBar;
    QLabel offsetBar;
    QLabel convertLabel;
    unsigned long startOffset;
    unsigned long lengthOffset;
    vector<QLabel*> floatingLabels;
    int prev_vscrollbar_value;
    int prev_hscrollbar_value;
    qint64 replaceBytes(QString searchText, QString replaceText, bool isHex = true);
    void saveSettings(string filePath);
    void closeEvent(QCloseEvent *event);
    void backgroundUpdateHexWidget();
    void clearBackgroundColor(QTableWidget *table);
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *e);
    bool loadFile(QString path, unsigned long start = 0, unsigned long offset = 0, bool updateUI=true);
    void keyPressEvent( QKeyEvent *event );
    void compare(QString filename);
    void addFloatingLabel(qint64 offset, int len, QString text, QString style = "", bool addComment = false);
    void clearFloatingLabels();
    void findPatterns();
    void saveDataToFile(string path, bool loadfile=false);
    void loadBinChart();
    void updateOffsetBar();
    void updateOffsetBarWithSelection();
    void chunkOpenFile(QString fpath = "");
    void updateUI();

public slots:
    void on_editor_mouse_click();
    void on_editor_mouse_move();
    void on_menu_file_open_click();
    void on_menu_file_chunk_open_click();
    void on_menu_file_diff_click();
    void on_menu_file_save_click();
    void on_menu_file_save_as_click();
    void on_search_button_click();
    void on_search_all_button_click();
    void on_replace_button_click();
    void on_convert_button_click();
    void on_replace_all_button_click();
    void on_menu_find_click();
    void on_menu_convert_bytes_click();
    void on_menu_file_new_window_click();
    void on_menu_file_quit_click();
    void on_back_search_button_click();
    void on_menu_goto_offset_click();
    void on_menu_open_text_viewer_click();
    void on_vertical_scrollbar_change(int value);
    void on_horizontal_scrollbar_change(int value);
    void on_menu_toggle_patterns_click();
    void on_menu_find_patterns_click();
    void on_menu_open_settings_click();
    void on_menu_open_patterns_click();
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
    void on_regex_checkbox_changed(int);

};

#endif // FHEX_H
