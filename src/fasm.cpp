#include "fasm.h"

Fasm::Fasm(QString bytes) : QMainWindow(nullptr)
{
    this->setWindowTitle("Fasm");
    this->setWindowIcon(QIcon("/usr/share/icons/fhex.png"));
    this->setMinimumSize(800, 500);
    textAsm = new QTextEdit(this);
    textOpcodes = new QTextEdit(this);
    textAsm->setStyleSheet(TEXTEDIT_STYLE);
    textOpcodes->setStyleSheet(TEXTEDIT_STYLE);
    this->textOpcodes->setPlainText(bytes);
    statusbar = new QLabel;
    QLabel *lblArch = new QLabel("Arch:");
    QLabel *lblMode = new QLabel("Mode:");
    QLabel *lblAsm = new QLabel("Assembly:");
    QLabel *lblOpcodes = new QLabel("Op codes:");
    cmbArch = new QComboBox;
    cmbArch->addItem("X86");
    cmbArch->addItem("ARM");
    cmbArch->addItem("ARM64");
    cmbArch->addItem("MIPS");
    cmbMode = new QComboBox;
    cmbMode->addItem("32 bit");
    cmbMode->addItem("64 bit");
    cmbMode->addItem("ARM");
    cmbMode->addItem("THUMB");
    cmbMode->addItem("MIPS32");
    cmbMode->addItem("MIPS64");
    QPushButton *setDisasm = new QPushButton("Disassemble");
    connect(setDisasm, &QPushButton::clicked, this, &Fasm::on_btnSetDisasm_click);
    QPushButton *setAsm = new QPushButton("Assemble");
    connect(setAsm, &QPushButton::clicked, this, &Fasm::on_btnSetAsm_click);
    QHBoxLayout *hbox = new QHBoxLayout;
    QHBoxLayout *buttons = new QHBoxLayout;
    QGridLayout *gridLayout = new QGridLayout;
    hbox->addWidget(lblArch, 0, Qt::AlignRight);
    hbox->addWidget(cmbArch, 0, Qt::AlignLeft);
    hbox->addWidget(lblMode, 0, Qt::AlignRight);
    hbox->addWidget(cmbMode, 0, Qt::AlignLeft);
    buttons->addWidget(setDisasm, 0, Qt::AlignRight);
    buttons->addWidget(setAsm, 0, Qt::AlignLeft);
    gridLayout->addLayout(hbox, 0, 0);
    gridLayout->addLayout(buttons, 0, 1);
    gridLayout->addWidget(lblAsm, 1, 0);
    gridLayout->addWidget(lblOpcodes, 1, 1);
    gridLayout->addWidget(textAsm, 2, 0);
    gridLayout->addWidget(textOpcodes, 2, 1);
    gridLayout->addWidget(statusbar, 3, 0, 1, 2);
    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(gridLayout);
    this->setCentralWidget(mainWidget);
    ks = nullptr;
    loadKeystone();
    loadCapstone();
}

void Fasm::loadCapstone() {
    updateArchMode();
    cs_opt_skipdata skipdata = {
       .mnemonic = "db",
    };
    cs_err err = cs_open(carch, cmode, &cs);
    if (err != CS_ERR_OK){
        this->statusbar->setText("Error: Failed capstone initialization");
    } else if (this->textOpcodes->toPlainText() != ""){
        size_t count;
        cs_insn *insn;
        cs_option(cs, CS_OPT_DETAIL, CS_OPT_ON);
        cs_option(cs, CS_OPT_SKIPDATA, CS_OPT_ON);
        cs_option(cs, CS_OPT_SKIPDATA_SETUP, (size_t)&skipdata);
        QByteArray bytes = QByteArray::fromHex(this->textOpcodes->toPlainText().toUtf8());
        char *t = bytes.data();
        count = cs_disasm(cs, reinterpret_cast<uint8_t*>(QByteArray::fromHex(this->textOpcodes->toPlainText().toUtf8()).data()), bytes.size(), 0, 0, &insn);
        if (count > 0) {
            this->statusbar->setText("Disassembled " + QString::number(count) + " instructions");
            this->textAsm->clear();
            size_t j;
            for (j = 0; j < count; j++) {
                this->textAsm->setPlainText(this->textAsm->toPlainText() + insn[j].mnemonic + " " + insn[j].op_str + ";\r\n");
            }
            cs_free(insn, count);
        } else {
            this->statusbar->setText("Error: Failed to disassemble given op codes\n");
        }
        cs_close(&cs);
    }
}

void Fasm::updateArchMode() {
    QString txtArch = cmbArch->currentText();
    QString txtMode = cmbMode->currentText();

    if (txtArch == "X86") {
        karch = KS_ARCH_X86;
        carch = CS_ARCH_X86;
    } else if (txtArch == "ARM") {
        karch = KS_ARCH_ARM;
        carch = CS_ARCH_ARM;
    } else if (txtArch == "ARM64") {
        karch = KS_ARCH_ARM64;
        carch = CS_ARCH_ARM64;
    } else if (txtArch == "MIPS") {
        karch = KS_ARCH_MIPS;
        carch = CS_ARCH_MIPS;
    }

    if (txtMode == "32 bit") {
        kmode = KS_MODE_32;
        cmode = CS_MODE_32;
    } else if (txtMode == "64 bit") {
        kmode = KS_MODE_64;
        cmode = CS_MODE_64;
    } else if (txtMode == "ARM") {
        kmode = KS_MODE_ARM;
        cmode = CS_MODE_ARM;
    } else if (txtMode == "THUMB") {
        kmode = KS_MODE_THUMB;
        cmode = CS_MODE_THUMB;
    } else if (txtMode == "MIPS32") {
        kmode = KS_MODE_MIPS32;
        cmode = CS_MODE_MIPS32;
    } else if (txtMode == "MIPS64") {
        kmode = KS_MODE_MIPS64;
        cmode = CS_MODE_MIPS64;
    }
}

void Fasm::loadKeystone() {
    if (ks != nullptr) {
        ks_close(ks);
        ks = nullptr;
    }
    updateArchMode();
    ks_err err;
    err = ks_open(karch, kmode, &ks);
    if (err != KS_ERR_OK) {
        statusbar->setText("Error: Failed keystone initialization");
    } else if (this->textAsm->toPlainText() != "") {
        this->textOpcodes->clear();
        QString assembly = this->textAsm->toPlainText();
        for (uint8_t b : asmToOpcodes(assembly)) {
            this->textOpcodes->setPlainText(this->textOpcodes->toPlainText() + QString::number(b, 16).rightJustified(2,'0'));
        }
    }
}

vector<uint8_t> Fasm::asmToOpcodes(QString &assembly) {
    vector<uint8_t> bytes;
    unsigned char *encode;
    size_t count;
    size_t size;
    if (ks_asm(ks, assembly.toStdString().c_str(), 0, &encode, &size, &count) != KS_ERR_OK) {
        this->statusbar->setText("Keystone error " + QString::number(ks_errno(ks)));
    } else {
        size_t i;
        for (i = 0; i < size; i++) {
            bytes.push_back(encode[i]);
        }
        this->statusbar->setText("Compiled: " + QString::number(size) + " bytes | Statements: " + QString::number(count) + " | Bytes: " + QString::number(bytes.size()));
      }

      ks_free(encode);
      return bytes;
}

Fasm::~Fasm() {
    ks_close(ks);
}

void Fasm::on_btnSetAsm_click() {
    this->loadKeystone();
}

void Fasm::on_btnSetDisasm_click() {
    this->loadCapstone();
}
