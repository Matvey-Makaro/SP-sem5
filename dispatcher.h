#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <QWidget>
#include <QPushButton>
#include <QString>
#include <QTableWidget>
#include <unordered_map>
#include <QTimer>

#include <Windows.h>

struct Process
{
    QString name;
    PROCESS_INFORMATION pi;
};

enum class Status
{
    PERFORMED,
    ENDED
};

class Dispatcher : public QWidget
{
    Q_OBJECT

public:
    Dispatcher(QWidget *parent = nullptr);
    ~Dispatcher();

private:
    void initUI();
    Status get_status(DWORD proc_id);

private slots:
    void run();
    void update_info();
    void close_process();

private:
    QPushButton* run_btn;
    QPushButton* close_btn;
    std::unordered_map<DWORD, Process> id_to_process;
    QTableWidget* table;
    QTimer* timer;
    int msec_for_update = 1000;
};
#endif // DISPATCHER_H
