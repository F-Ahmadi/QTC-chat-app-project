#ifndef MYCOONECTDIALOG_H
#define MYCOONECTDIALOG_H


#include <QMessageBox>
#include <QDialog>
#include <QString>


namespace Ui {
class MycoonectDialog;
}

class MycoonectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MycoonectDialog(QWidget *parent = nullptr);
    ~MycoonectDialog();
    QString returnmyusername() const ;
    quint64 returnport() const;

private slots:
    void on_okbtn_clicked();

    void on_cancelbtn_clicked();

private:
    Ui::MycoonectDialog *ui;
    QString myusername;
    quint64 myport;
public:


};

inline QString MycoonectDialog::returnmyusername() const
{
    return myusername;
}

inline quint64 MycoonectDialog::returnport() const
{
    return myport;
}

#endif // MYCOONECTDIALOG_H
