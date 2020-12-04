#ifndef FASM_H
#define FASM_H

#include <QMainWindow>
#include <QWidget>
#include <QGridLayout>
#include <QTextEdit>
#include <QDebug>
#include <keystone/keystone.h>
#include <capstone/capstone.h>
#include <QLabel>
#include <vector>
#include <QComboBox>
#include <QPushButton>
#include <QByteArray>

#define TEXTEDIT_STYLE "QTextEdit { background-color: #17120f; color: #ebe5e1; font-size: 16px; font-family: monospace; }"

using namespace std;

class Fasm : public QMainWindow
{
    Q_OBJECT
private:
    QComboBox *cmbArch;
    QComboBox *cmbMode;
    QLabel *statusbar;
    ks_engine *ks;
    csh cs;
    QTextEdit *textAsm;
    QTextEdit *textOpcodes;
    ks_arch karch;
    ks_mode kmode;
    cs_arch carch;
    cs_mode cmode;
    vector<uint8_t> asmToOpcodes(QString &assembly);
    void loadKeystone();
    void loadCapstone();
    void updateArchMode();
public:
    Fasm(QString bytes);
    ~Fasm();

public slots:
    void on_btnSetDisasm_click();
    void on_btnSetAsm_click();
};

#endif // FASM_H
