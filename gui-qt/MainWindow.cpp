#include "MainWindow.h"
#include "CPUBridge.h"
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), bridge(new CPUBridge(this)) {
    // Central widget
    central = new QWidget(this);
    setCentralWidget(central);

    // Left column: registers and controls
    QVBoxLayout* left = new QVBoxLayout();

    // Registers group
    QGroupBox* regs = new QGroupBox("Registers");
    QGridLayout* regsLayout = new QGridLayout();
    regs->setLayout(regsLayout);

    regsLayout->addWidget(new QLabel("PC:"), 0, 0);
    pcLabel = new QLabel("0x0000"); regsLayout->addWidget(pcLabel, 0, 1);
    regsLayout->addWidget(new QLabel("SP:"), 1, 0);
    spLabel = new QLabel("0x0000"); regsLayout->addWidget(spLabel, 1, 1);
    regsLayout->addWidget(new QLabel("X:"), 2, 0);
    xLabel = new QLabel("0x0000"); regsLayout->addWidget(xLabel, 2, 1);
    regsLayout->addWidget(new QLabel("Y:"), 3, 0);
    yLabel = new QLabel("0x0000"); regsLayout->addWidget(yLabel, 3, 1);

    regsLayout->addWidget(new QLabel("C:"), 4, 0);
    carryLabel = new QLabel("0"); regsLayout->addWidget(carryLabel, 4, 1);
    regsLayout->addWidget(new QLabel("Z:"), 5, 0);
    zeroLabel = new QLabel("0"); regsLayout->addWidget(zeroLabel, 5, 1);
    regsLayout->addWidget(new QLabel("I:"), 6, 0);
    intLabel = new QLabel("0"); regsLayout->addWidget(intLabel, 6, 1);

    left->addWidget(regs);

    // Controls
    QGroupBox* controls = new QGroupBox("Controls");
    QHBoxLayout* ctrlLayout = new QHBoxLayout();
    controls->setLayout(ctrlLayout);
    stepButton = new QPushButton("Step");
    runButton = new QPushButton("Run");
    stopButton = new QPushButton("Stop");
    resetButton = new QPushButton("Reset");
    ctrlLayout->addWidget(stepButton);
    ctrlLayout->addWidget(runButton);
    ctrlLayout->addWidget(stopButton);
    ctrlLayout->addWidget(resetButton);
    left->addWidget(controls);

    // LEDs
    QGroupBox* ledsBox = new QGroupBox("GPIO LEDs");
    QHBoxLayout* ledsLayout = new QHBoxLayout();
    ledsBox->setLayout(ledsLayout);
    for (int i = 0; i < 8; ++i) {
        QLabel* l = new QLabel();
        l->setFixedSize(24,24);
        l->setStyleSheet("background: #444; border-radius: 12px;");
        ledsLayout->addWidget(l);
        ledLabels.push_back(l);
    }
    left->addWidget(ledsBox);

    // Right: memory and code (placeholder)
    QVBoxLayout* right = new QVBoxLayout();
    QGroupBox* memBox = new QGroupBox("Memory (partial view)");
    QVBoxLayout* memLayout = new QVBoxLayout();
    memBox->setLayout(memLayout);
    QLabel* memPlaceholder = new QLabel("Memory view will be implemented here.");
    memLayout->addWidget(memPlaceholder);
    right->addWidget(memBox);

    // Layout
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->addLayout(left, 0);
    mainLayout->addLayout(right, 1);

    // CPU Bridge
    bridge->initialize();

    connect(stepButton, &QPushButton::clicked, this, &MainWindow::onStep);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::onRun);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::onReset);

    connect(bridge, &CPUBridge::registersChanged, this, &MainWindow::updateRegisters);
    connect(bridge, &CPUBridge::gpioChanged, this, [this](uint8_t v){
        for (int i = 0; i < 8 && i < (int)ledLabels.size(); ++i) {
            bool on = (v & (1<<i)) != 0;
            ledLabels[i]->setStyleSheet(on ? "background: red; border-radius: 12px;" : "background: #444; border-radius: 12px;");
        }
    });

    pollTimer = new QTimer(this);
    pollTimer->setInterval(100);
    connect(pollTimer, &QTimer::timeout, this, &MainWindow::updateRegisters);
    pollTimer->start();
}

MainWindow::~MainWindow() {
}

void MainWindow::onStep() { bridge->step(); }
void MainWindow::onRun() { bridge->run(); }
void MainWindow::onStop() { bridge->stop(); }
void MainWindow::onReset() { bridge->reset(); }

void MainWindow::updateRegisters() {
    pcLabel->setText(QString("0x%1").arg(bridge->getPC(), 4, 16, QChar('0')));
    spLabel->setText(QString("0x%1").arg(bridge->getSP(), 4, 16, QChar('0')));
    xLabel->setText(QString("0x%1").arg(bridge->getX(), 4, 16, QChar('0')));
    yLabel->setText(QString("0x%1").arg(bridge->getY(), 4, 16, QChar('0')));
    carryLabel->setText(bridge->getCarry() ? "1" : "0");
    zeroLabel->setText(bridge->getZero() ? "1" : "0");
    intLabel->setText(bridge->getInterrupt() ? "1" : "0");
}
