#pragma once

#include <QMainWindow>

class CPUBridge;

class QLabel;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onStep();
    void onRun();
    void onStop();
    void onReset();
    void updateRegisters();

private:
    // UI widgets
    QWidget* central;
    QLabel* pcLabel;
    QLabel* spLabel;
    QLabel* xLabel;
    QLabel* yLabel;
    QLabel* carryLabel;
    QLabel* zeroLabel;
    QLabel* intLabel;
    std::vector<QLabel*> ledLabels;
    QPushButton* stepButton;
    QPushButton* runButton;
    QPushButton* stopButton;
    QPushButton* resetButton;

    // Bridge to CPU
    CPUBridge* bridge;
    QTimer* pollTimer;
};
