#include "mainwindow.h"
#include "instrumentcanvas.h"

#include <QAction>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDrag>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QUuid>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QSet>
#include <QAbstractItemView>
#include <QInputDialog>
#include <QScrollArea>
#include <QColor>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    buildUi();
    buildMenus();

    InstrumentElement knob;
    knob.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    knob.label = "Cutoff";
    knob.parameter = "filter.cutoff";
    knob.x = 45; knob.y = 45;

    InstrumentElement slider;
    slider.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    slider.type = "slider";
    slider.label = "Resonance";
    slider.parameter = "filter.resonance";
    slider.x = 170; slider.y = 45;
    slider.width = 120; slider.height = 80;

    m_document.elements = {knob, slider};
    refreshCanvas();
    refreshElementList();
    selectElement(knob.id);
    setWindowTitle("SynthEditor — Untitled");
    resize(1250, 760);
}

void MainWindow::buildUi()
{
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    // Left panel: draggable widget library + element tree.
    auto *left = new QWidget(splitter);
    auto *leftLayout = new QVBoxLayout(left);
    leftLayout->addWidget(new QLabel("BIBLIOTEKA ELEMENTÓW", left));
    auto *library = new QListWidget(left);
    library->addItems({"Pokrętło", "Suwak", "Przycisk"});
    library->setDragEnabled(true);
    library->setDragDropMode(QAbstractItemView::DragOnly);
    library->setMaximumHeight(110);
    connect(library, &QListWidget::itemPressed, this, [library](QListWidgetItem *item) {
        auto *mime = new QMimeData;
        const QString label = item->text();
        const QString type = label == "Pokrętło" ? "knob" :
                             label == "Suwak" ? "slider" : "button";
        mime->setData("application/x-syntheditor-element", type.toUtf8());
        auto *drag = new QDrag(library);
        drag->setMimeData(mime);
        drag->exec(Qt::CopyAction);
    });
    leftLayout->addWidget(library);
    leftLayout->addWidget(new QLabel("ELEMENTY W INSTRUMENCIE", left));
    m_elementList = new QListWidget(left);
    leftLayout->addWidget(m_elementList, 1);
    auto *removeButton = new QPushButton("Usuń zaznaczony element", left);
    leftLayout->addWidget(removeButton);

    // Center: layout canvas.
    auto *center = new QWidget(splitter);
    auto *centerLayout = new QVBoxLayout(center);
    centerLayout->addWidget(new QLabel("OBSZAR ROBOCZY — przeciągnij elementy z biblioteki", center));
    m_canvas = new InstrumentCanvas(center);
    centerLayout->addWidget(m_canvas, 1);

    // Right panel: scrollable property inspector.
    auto *right = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(right);
    rightLayout->addWidget(new QLabel("INSPEKTOR WŁAŚCIWOŚCI", right));
    auto *scroll = new QScrollArea(right);
    scroll->setWidgetResizable(true);
    auto *props = new QWidget(scroll);
    auto *form = new QFormLayout(props);

    m_nameEdit = new QLineEdit(props);
    m_typeCombo = new QComboBox(props);
    m_typeCombo->addItems({"knob", "slider", "button"});
    m_parameterEdit = new QLineEdit(props);

    auto intSpin = [props](int lo, int hi) {
        auto *s = new QSpinBox(props); s->setRange(lo, hi); return s;
    };
    m_xSpin = intSpin(0, 4000);
    m_ySpin = intSpin(0, 3000);
    m_widthSpin = intSpin(20, 1000);
    m_heightSpin = intSpin(20, 1000);
    m_fontSizeSpin = intSpin(6, 48);

    auto doubleSpin = [props](double lo, double hi, double val) {
        auto *s = new QDoubleSpinBox(props);
        s->setRange(lo, hi); s->setDecimals(4); s->setValue(val); return s;
    };
    m_minSpin = doubleSpin(-1e6, 1e6, 0);
    m_maxSpin = doubleSpin(-1e6, 1e6, 1);
    m_valueSpin = doubleSpin(-1e6, 1e6, .5);

    m_colorEdit = new QLineEdit(props);
    auto *colorButton = new QPushButton("Wybierz…", props);
    auto *colorRow = new QWidget(props);
    auto *colorRowLayout = new QHBoxLayout(colorRow);
    colorRowLayout->setContentsMargins(0, 0, 0, 0);
    colorRowLayout->addWidget(m_colorEdit, 1);
    colorRowLayout->addWidget(colorButton);
    m_textColorEdit = new QLineEdit(props);

    form->addRow("Etykieta", m_nameEdit);
    form->addRow("Typ", m_typeCombo);
    form->addRow("Parametr silnika", m_parameterEdit);
    form->addRow("X", m_xSpin);
    form->addRow("Y", m_ySpin);
    form->addRow("Szerokość", m_widthSpin);
    form->addRow("Wysokość", m_heightSpin);
    form->addRow("Minimum", m_minSpin);
    form->addRow("Maksimum", m_maxSpin);
    form->addRow("Wartość", m_valueSpin);
    form->addRow("Kolor akcentu", colorRow);
    form->addRow("Kolor tekstu", m_textColorEdit);
    form->addRow("Rozmiar czcionki", m_fontSizeSpin);

    form->addRow(new QLabel("TŁO I ROZMIAR PANELU", props));
    m_bgColorEdit = new QLineEdit(props);
    auto *bgColorButton = new QPushButton("Wybierz…", props);
    auto *bgColorRow = new QWidget(props);
    auto *bgColorLayout = new QHBoxLayout(bgColorRow);
    bgColorLayout->setContentsMargins(0, 0, 0, 0);
    bgColorLayout->addWidget(m_bgColorEdit, 1);
    bgColorLayout->addWidget(bgColorButton);
    m_bgImageEdit = new QLineEdit(props);
    auto *imageButton = new QPushButton("Wybierz obraz…", props);
    auto *imageRow = new QWidget(props);
    auto *imageLayout = new QHBoxLayout(imageRow);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->addWidget(m_bgImageEdit, 1);
    imageLayout->addWidget(imageButton);
    m_canvasWidthSpin = intSpin(200, 4000);
    m_canvasHeightSpin = intSpin(150, 3000);
    form->addRow("Kolor tła", bgColorRow);
    form->addRow("Obraz tła", imageRow);
    form->addRow("Szerokość panelu", m_canvasWidthSpin);
    form->addRow("Wysokość panelu", m_canvasHeightSpin);

    scroll->setWidget(props);
    rightLayout->addWidget(scroll, 1);

    splitter->addWidget(left);
    splitter->addWidget(center);
    splitter->addWidget(right);
    splitter->setSizes({220, 720, 300});
    setCentralWidget(splitter);

    m_statusLabel = new QLabel("Gotowy", this);
    statusBar()->addWidget(m_statusLabel);

    connect(removeButton, &QPushButton::clicked, this, [this] {
        const QString id = m_canvas->selectedId();
        if (id.isEmpty()) return;
        for (qsizetype i = 0; i < m_document.elements.size(); ++i) {
            if (m_document.elements.at(i).id == id) {
                m_document.elements.removeAt(i);
                break;
            }
        }
        refreshCanvas(); refreshElementList(); selectElement({});
    });
    connect(m_elementList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < m_document.elements.size())
            selectElement(m_document.elements.at(row).id);
    });
    connect(m_canvas, &InstrumentCanvas::elementSelected, this, &MainWindow::selectElement);
    connect(m_canvas, &InstrumentCanvas::elementDropped, this,
            [this](const QString &type, int x, int y) { addElement(type, x, y); });
    connect(m_canvas, &InstrumentCanvas::elementMoved, this,
            [this](const QString &id, int x, int y) {
        for (auto &e : m_document.elements) if (e.id == id) { e.x = x; e.y = y; break; }
        refreshCanvas(); refreshElementList();
    });
    connect(m_canvas, &InstrumentCanvas::elementResized, this,
            [this](const QString &id, int x, int y, int w, int h) {
        for (auto &e : m_document.elements) {
            if (e.id == id) { e.x = x; e.y = y; e.width = w; e.height = h; break; }
        }
        refreshCanvas(); refreshElementList(); selectElement(id);
    });

    auto bindEditors = [this](QObject *obj) {
        if (auto *line = qobject_cast<QLineEdit *>(obj))
            connect(line, &QLineEdit::editingFinished, this, &MainWindow::updateSelectedElementFromProperties);
        else if (auto *combo = qobject_cast<QComboBox *>(obj))
            connect(combo, &QComboBox::activated, this, [this](int) { updateSelectedElementFromProperties(); });
        else if (auto *spin = qobject_cast<QSpinBox *>(obj))
            connect(spin, &QSpinBox::editingFinished, this, &MainWindow::updateSelectedElementFromProperties);
        else if (auto *spin = qobject_cast<QDoubleSpinBox *>(obj))
            connect(spin, &QDoubleSpinBox::editingFinished, this, &MainWindow::updateSelectedElementFromProperties);
    };
    const QList<QObject *> elementEditors = {
        m_nameEdit, m_typeCombo, m_parameterEdit, m_xSpin, m_ySpin,
        m_widthSpin, m_heightSpin, m_minSpin, m_maxSpin, m_valueSpin,
        m_colorEdit, m_textColorEdit, m_fontSizeSpin
    };
    for (QObject *o : elementEditors) bindEditors(o);

    connect(colorButton, &QPushButton::clicked, this, [this] {
        auto *e = selectedElement(); if (!e) return;
        QColor color(e->color);
        color = QColorDialog::getColor(color, this, "Kolor kontrolki");
        if (color.isValid()) { m_colorEdit->setText(color.name()); updateSelectedElementFromProperties(); }
    });
    connect(bgColorButton, &QPushButton::clicked, this, [this] {
        QColor color(m_document.backgroundColor);
        color = QColorDialog::getColor(color, this, "Kolor tła");
        if (color.isValid()) { m_bgColorEdit->setText(color.name()); updateCanvasStyle(); }
    });
    connect(imageButton, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(this, "Wybierz obraz tła", {},
            "Obrazy (*.png *.jpg *.jpeg *.bmp *.webp)");
        if (!path.isEmpty()) { m_bgImageEdit->setText(path); updateCanvasStyle(); }
    });
    connect(m_bgColorEdit, &QLineEdit::editingFinished, this, &MainWindow::updateCanvasStyle);
    connect(m_bgImageEdit, &QLineEdit::editingFinished, this, &MainWindow::updateCanvasStyle);
    connect(m_canvasWidthSpin, &QSpinBox::editingFinished, this, &MainWindow::updateCanvasStyle);
    connect(m_canvasHeightSpin, &QSpinBox::editingFinished, this, &MainWindow::updateCanvasStyle);
}

void MainWindow::buildMenus()
{
    auto *file = menuBar()->addMenu("&Plik");
    auto *newAction = file->addAction("&Nowy");
    auto *openAction = file->addAction("&Otwórz...");
    auto *saveAction = file->addAction("&Zapisz");
    auto *saveAsAction = file->addAction("Zapisz &jako...");
    file->addSeparator();
    auto *exitAction = file->addAction("Zakończ");

    connect(newAction, &QAction::triggered, this, [this] {
        m_document = InstrumentDocument{};
        m_currentFile.clear();
        refreshCanvas(); refreshElementList(); selectElement({});
        m_bgColorEdit->setText(m_document.backgroundColor);
        m_bgImageEdit->clear();
        m_canvasWidthSpin->setValue(m_document.canvasWidth);
        m_canvasHeightSpin->setValue(m_document.canvasHeight);
        updateCanvasStyle();
        setWindowTitle("SynthEditor — Untitled");
        m_statusLabel->setText("Nowy instrument");
    });
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto *toolbar = addToolBar("Plik");
    toolbar->addAction(newAction); toolbar->addAction(openAction); toolbar->addAction(saveAction);
}

void MainWindow::addElement(const QString &type, int x, int y)
{
    InstrumentElement e;
    e.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    e.type = type;
    e.label = type == "knob" ? "Nowe pokrętło" : type == "slider" ? "Nowy suwak" : "Nowy przycisk";
    e.parameter = "instrument.parameter_" + QString::number(m_document.elements.size() + 1);
    e.x = x >= 0 ? x : 30 + (m_document.elements.size() % 5) * 110;
    e.y = y >= 0 ? y : 30 + (m_document.elements.size() / 5) * 110;
    if (type == "slider") { e.width = 120; e.height = 70; }
    if (type == "button") { e.width = 110; e.height = 65; }
    m_document.elements.append(e);
    refreshCanvas(); refreshElementList(); selectElement(e.id);
}

void MainWindow::refreshCanvas()
{
    m_canvas->setElements(m_document.elements);
    m_canvas->setCanvasLogicalSize(m_document.canvasWidth, m_document.canvasHeight);
    m_canvas->setCanvasBackground(m_document.backgroundColor, m_document.backgroundImage);
}

void MainWindow::refreshElementList()
{
    const QString selected = m_canvas->selectedId();
    m_elementList->clear();
    for (const auto &e : m_document.elements) {
        auto *item = new QListWidgetItem(e.label + " [" + e.type + "]", m_elementList);
        item->setData(Qt::UserRole, e.id);
        if (e.id == selected) m_elementList->setCurrentItem(item);
    }
}

InstrumentElement *MainWindow::selectedElement()
{
    const QString id = m_canvas->selectedId();
    for (auto &e : m_document.elements) if (e.id == id) return &e;
    return nullptr;
}

void MainWindow::selectElement(const QString &id)
{
    m_canvas->setSelectedId(id);
    for (int i = 0; i < m_elementList->count(); ++i) {
        auto *item = m_elementList->item(i);
        if (item->data(Qt::UserRole).toString() == id) {
            m_elementList->setCurrentItem(item);
            break;
        }
    }
    auto *e = selectedElement();
    const QList<QWidget *> editors = {
        m_nameEdit, m_typeCombo, m_parameterEdit, m_xSpin, m_ySpin,
        m_widthSpin, m_heightSpin, m_minSpin, m_maxSpin, m_valueSpin,
        m_colorEdit, m_textColorEdit, m_fontSizeSpin
    };
    for (QWidget *w : editors) w->setEnabled(e != nullptr);
    if (!e) return;
    m_nameEdit->setText(e->label);
    m_typeCombo->setCurrentText(e->type);
    m_parameterEdit->setText(e->parameter);
    m_xSpin->setValue(e->x); m_ySpin->setValue(e->y);
    m_widthSpin->setValue(e->width); m_heightSpin->setValue(e->height);
    m_minSpin->setValue(e->minimum); m_maxSpin->setValue(e->maximum);
    m_valueSpin->setValue(e->value);
    m_colorEdit->setText(e->color); m_textColorEdit->setText(e->textColor);
    m_fontSizeSpin->setValue(e->fontSize);
}

void MainWindow::updateSelectedElementFromProperties()
{
    auto *e = selectedElement();
    if (!e) return;
    const QString label = m_nameEdit->text().trimmed();
    const QString parameter = m_parameterEdit->text().trimmed();
    const QColor accent(m_colorEdit->text());
    const QColor textColor(m_textColorEdit->text());
    if (label.isEmpty() || parameter.isEmpty()) {
        m_statusLabel->setText("Etykieta i parametr nie mogą być puste.");
        return;
    }
    if (!accent.isValid() || !textColor.isValid()) {
        m_statusLabel->setText("Kolory muszą być zapisane w formacie np. #61a8ff.");
        return;
    }
    e->label = label; e->type = m_typeCombo->currentText(); e->parameter = parameter;
    e->x = m_xSpin->value(); e->y = m_ySpin->value();
    e->width = m_widthSpin->value(); e->height = m_heightSpin->value();
    e->minimum = m_minSpin->value(); e->maximum = m_maxSpin->value();
    if (e->maximum <= e->minimum) {
        m_statusLabel->setText("Maksimum musi być większe niż minimum.");
        return;
    }
    e->value = qBound(e->minimum, m_valueSpin->value(), e->maximum);
    e->color = accent.name(); e->textColor = textColor.name();
    e->fontSize = m_fontSizeSpin->value();
    refreshCanvas(); refreshElementList();
    m_statusLabel->setText("Zmieniono właściwości");
}

void MainWindow::updateCanvasStyle()
{
    const QColor color(m_bgColorEdit->text());
    if (!color.isValid()) {
        m_statusLabel->setText("Niepoprawny kolor tła.");
        return;
    }
    const QString image = m_bgImageEdit->text().trimmed();
    if (!image.isEmpty() && !QFileInfo::exists(image)) {
        m_statusLabel->setText("Plik obrazu tła nie istnieje.");
        return;
    }
    m_document.backgroundColor = color.name();
    m_document.backgroundImage = image;
    m_document.canvasWidth = m_canvasWidthSpin->value();
    m_document.canvasHeight = m_canvasHeightSpin->value();
    refreshCanvas();
    m_statusLabel->setText("Zmieniono tło lub rozmiar panelu");
}

bool MainWindow::validateDocument(const InstrumentDocument &doc, QString *error) const
{
    if (doc.name.trimmed().isEmpty()) {
        *error = "Nazwa instrumentu nie może być pusta."; return false;
    }
    if (doc.canvasWidth < 200 || doc.canvasHeight < 150) {
        *error = "Panel jest zbyt mały."; return false;
    }
    if (!QColor(doc.backgroundColor).isValid()) {
        *error = "Niepoprawny kolor tła."; return false;
    }
    if (!doc.backgroundImage.isEmpty() && !QFileInfo::exists(doc.backgroundImage)) {
        *error = "Nie znaleziono obrazu tła: " + doc.backgroundImage; return false;
    }

    QSet<QString> ids;
    QSet<QString> parameters;
    const QRegularExpression paramRegex("^[A-Za-z_][A-Za-z0-9_.-]*$");
    for (const auto &e : doc.elements) {
        if (e.id.trimmed().isEmpty() || ids.contains(e.id)) {
            *error = "Każdy element musi mieć unikalne ID."; return false;
        }
        ids.insert(e.id);
        if (e.type != "knob" && e.type != "slider" && e.type != "button") {
            *error = "Nieobsługiwany typ elementu: " + e.type; return false;
        }
        if (e.label.trimmed().isEmpty()) {
            *error = "Etykieta elementu nie może być pusta."; return false;
        }
        if (!paramRegex.match(e.parameter).hasMatch()) {
            *error = "Niepoprawna nazwa parametru: " + e.parameter; return false;
        }
        if (parameters.contains(e.parameter)) {
            *error = "Parametr musi być unikalny: " + e.parameter; return false;
        }
        parameters.insert(e.parameter);
        if (e.width < 20 || e.height < 20 || e.x < 0 || e.y < 0 ||
            e.x + e.width > doc.canvasWidth || e.y + e.height > doc.canvasHeight) {
            *error = "Element „" + e.label + "” wychodzi poza panel lub ma niepoprawny rozmiar."; return false;
        }
        if (e.maximum <= e.minimum || e.value < e.minimum || e.value > e.maximum) {
            *error = "Niepoprawny zakres wartości elementu „" + e.label + "”."; return false;
        }
        if (!QColor(e.color).isValid() || !QColor(e.textColor).isValid()) {
            *error = "Niepoprawny kolor elementu „" + e.label + "”."; return false;
        }
    }
    return true;
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(this, "Otwórz instrument", {},
        "Instrumenty SynthEditor (*.dat *.json);;Wszystkie pliki (*)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku:\n" + file.errorString());
        return;
    }
    QJsonParseError parseError;
    const QJsonDocument json = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !json.isObject()) {
        QMessageBox::critical(this, "Błąd pliku", "Niepoprawny JSON:\n" + parseError.errorString());
        return;
    }
    if (json.object().value("formatVersion").toInt() != 1) {
        QMessageBox::warning(this, "Wersja formatu", "Nieobsługiwana wersja pliku instrumentu.");
        return;
    }
    InstrumentDocument loaded = InstrumentDocument::fromJson(json.object());
    QString error;
    if (!validateDocument(loaded, &error)) {
        QMessageBox::critical(this, "Niepoprawny instrument", error);
        return;
    }
    m_document = loaded;
    setCurrentFile(path);
    m_bgColorEdit->setText(m_document.backgroundColor);
    m_bgImageEdit->setText(m_document.backgroundImage);
    m_canvasWidthSpin->setValue(m_document.canvasWidth);
    m_canvasHeightSpin->setValue(m_document.canvasHeight);
    refreshCanvas(); refreshElementList(); selectElement({});
    m_statusLabel->setText("Wczytano instrument");
}

void MainWindow::saveFile()
{
    if (m_currentFile.isEmpty()) { saveFileAs(); return; }
    saveToPath(m_currentFile);
}

void MainWindow::saveFileAs()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, "Nazwa instrumentu",
        "Nazwa widoczna w DAW:", QLineEdit::Normal, m_document.name, &ok);
    if (!ok) return;
    m_document.name = name.trimmed();
    const QString suggested = m_document.name.isEmpty() ? "instrument.dat" :
        m_document.name.replace(' ', '_') + ".dat";
    const QString path = QFileDialog::getSaveFileName(this, "Zapisz instrument", suggested,
        "Instrumenty SynthEditor (*.dat);;Pliki JSON (*.json)");
    if (path.isEmpty()) return;
    saveToPath(path);
}

bool MainWindow::saveToPath(const QString &path)
{
    QString error;
    if (!validateDocument(m_document, &error)) {
        QMessageBox::warning(this, "Nie można zapisać instrumentu", error);
        return false;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::critical(this, "Błąd zapisu", file.errorString());
        return false;
    }
    const QByteArray data = QJsonDocument(m_document.toJson()).toJson(QJsonDocument::Indented);
    if (file.write(data) != data.size()) {
        QMessageBox::critical(this, "Błąd zapisu", "Nie udało się zapisać całego pliku.");
        return false;
    }
    setCurrentFile(path);
    m_statusLabel->setText("Zapisano: " + QFileInfo(path).fileName());
    return true;
}

void MainWindow::setCurrentFile(const QString &path)
{
    m_currentFile = path;
    setWindowTitle("SynthEditor — " + QFileInfo(path).fileName());
}
