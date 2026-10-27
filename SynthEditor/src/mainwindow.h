#pragma once

#include "instrumentmodel.h"
#include <QMainWindow>

class InstrumentCanvas;
class QListWidget;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void buildUi();
    void buildMenus();
    void addElement(const QString &type, int x = -1, int y = -1);
    void refreshCanvas();
    void refreshElementList();
    void selectElement(const QString &id);
    InstrumentElement *selectedElement();
    void updateSelectedElementFromProperties();
    void openFile();
    void saveFile();
    void saveFileAs();
    bool saveToPath(const QString &path);
    void setCurrentFile(const QString &path);
    bool validateDocument(const InstrumentDocument &doc, QString *error) const;
    void updateCanvasStyle();

    InstrumentDocument m_document;
    QString m_currentFile;
    InstrumentCanvas *m_canvas = nullptr;
    QListWidget *m_elementList = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QLineEdit *m_parameterEdit = nullptr;
    QComboBox *m_typeCombo = nullptr;
    QSpinBox *m_xSpin = nullptr;
    QSpinBox *m_ySpin = nullptr;
    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
    QDoubleSpinBox *m_minSpin = nullptr;
    QDoubleSpinBox *m_maxSpin = nullptr;
    QDoubleSpinBox *m_valueSpin = nullptr;
    QLineEdit *m_colorEdit = nullptr;
    QLineEdit *m_textColorEdit = nullptr;
    QSpinBox *m_fontSizeSpin = nullptr;
    QLineEdit *m_bgColorEdit = nullptr;
    QLineEdit *m_bgImageEdit = nullptr;
    QSpinBox *m_canvasWidthSpin = nullptr;
    QSpinBox *m_canvasHeightSpin = nullptr;
    QLabel *m_statusLabel = nullptr;
};
