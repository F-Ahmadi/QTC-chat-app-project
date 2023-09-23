#include "mycoonectdialog.h"
#include "ui_mycoonectdialog.h"




MycoonectDialog::MycoonectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MycoonectDialog)
{
    ui->setupUi(this);
}

MycoonectDialog::~MycoonectDialog()
{
    delete ui;
}

void MycoonectDialog::on_okbtn_clicked()
{
    myport = ui->spinBox->value();
    myusername = ui->username_lineEdit->text();

    if(myusername.isEmpty()){
        QMessageBox::critical(this,"Error", QString("don't enter name!"));
        return;
    }
   accept();

}// end of on_okbtn_clicked()


void MycoonectDialog::on_cancelbtn_clicked()
{
    exit(EXIT_FAILURE);
}

