#include "dispatcher.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>

BOOL CALLBACK kill_app_enum_windows(IN HWND hWnd, IN LPARAM lParam)
{
    assert(lParam != NULL);
    DWORD window_proc_id;
    GetWindowThreadProcessId(hWnd, &window_proc_id);

    if(IsWindowVisible(hWnd) && window_proc_id == (DWORD)lParam)
        PostMessage(hWnd, WM_CLOSE, 0, 0);

    return TRUE;
}

Dispatcher::Dispatcher(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void Dispatcher::initUI()
{
    run_btn = new QPushButton("Run", this);
    close_btn = new QPushButton("Close", this);

    table = new QTableWidget(0, 3, this);
    QStringList lst;
    lst << "id" << "name" << "status";
    table->setHorizontalHeaderLabels(lst);
    table->setEditTriggers(0);

    auto* vbox = new QVBoxLayout(this);
    auto* hbox = new QHBoxLayout();
    vbox->addWidget(table);
    vbox->addLayout(hbox);

    hbox->addWidget(run_btn);
    hbox->addWidget(close_btn);

    timer = new QTimer(this);

    connect(run_btn, &QPushButton::clicked, this, &Dispatcher::run);
    connect(close_btn, &QPushButton::clicked, this, &Dispatcher::close_process);
    connect(timer, &QTimer::timeout, this, &Dispatcher::update_info);

    timer->start(msec_for_update);
}

Status Dispatcher::get_status(DWORD proc_id)
{
    DWORD status = WaitForSingleObject(id_to_process[proc_id].pi.hProcess, 0);
    if(status == WAIT_OBJECT_0)
        return Status::ENDED;
    return Status::PERFORMED;

//    DWORD exit_code;
//    GetExitCodeProcess(id_to_process[proc_id].pi.hProcess, &exit_code);
//    if(exit_code == STILL_ACTIVE)
//        return Status::PERFORMED;
//    return Status::ENDED;
}

void Dispatcher::run()
{
    auto fileName = QFileDialog::getOpenFileName(this, "Select exe.", "", "*.exe");
    if(fileName.isEmpty())
        return;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    if (!CreateProcess(
      NULL,
      (LPWSTR)fileName.toStdWString().c_str(),
      NULL,
      NULL,
      FALSE,
      0,
      NULL,
      NULL,
      &si,
      &pi))
    {
       QMessageBox::warning(this, "Warning", "Process \"" + fileName + "\" did not start");
       return;
    }

    id_to_process[pi.dwProcessId] = {fileName, pi};

    auto* const id_item = new QTableWidgetItem(QString("%1").arg(pi.dwProcessId));
    auto* const name_item = new QTableWidgetItem(fileName);
    auto* const status_item = new QTableWidgetItem("Performed");
    const auto curr_row = table->rowCount();

    table->setRowCount(table->rowCount() + 1);
    table->setItem(curr_row, 0, id_item);
    table->setItem(curr_row, 1, name_item);
    table->setItem(curr_row, 2, status_item);
}

void Dispatcher::update_info()
{
    for(int i = 0; i < table->rowCount(); i++)
    {
        auto id = static_cast<DWORD>(table->item(i, 0)->text().toUInt());
        auto status = get_status(id);
        auto* const curr_item = table->item(i, 2);
        if(status == Status::PERFORMED)
            curr_item->setText("Performed");
        else curr_item->setText("Ended");
    }

    timer->start(msec_for_update);
}

void Dispatcher::close_process()
{
    auto curr_row = table->currentRow();
    if(curr_row == -1)
    {
        QMessageBox::warning(this, "Warning", "Process not selected.");
        return;
    }

    auto status = table->item(curr_row, 2)->text();
    if(status == "Ended")
    {
        QMessageBox::warning(this, "Warning", "Process already closed.");
        return;
    }

    auto id = static_cast<DWORD>(table->item(curr_row, 0)->text().toUInt());
    const auto& process = id_to_process[id];

    EnumWindows(kill_app_enum_windows, process.pi.dwProcessId);


}

Dispatcher::~Dispatcher()
{
}

