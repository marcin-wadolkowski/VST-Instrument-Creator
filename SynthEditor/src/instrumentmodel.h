#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

struct InstrumentElement
{
    QString id;
    QString type = "knob";
    QString label = "Cutoff";
    QString parameter = "filter.cutoff";
    int x = 40;
    int y = 40;
    int width = 90;
    int height = 90;
    double minimum = 0.0;
    double maximum = 1.0;
    double value = 0.5;
    QString color = "#61a8ff";
    QString textColor = "#edf2f8";
    int fontSize = 10;

    QJsonObject toJson() const
    {
        return {
            {"id", id}, {"type", type}, {"label", label},
            {"parameter", parameter}, {"x", x}, {"y", y},
            {"width", width}, {"height", height},
            {"min", minimum}, {"max", maximum}, {"value", value},
            {"color", color}, {"textColor", textColor}, {"fontSize", fontSize}
        };
    }

    static InstrumentElement fromJson(const QJsonObject &obj)
    {
        InstrumentElement e;
        e.id = obj.value("id").toString();
        e.type = obj.value("type").toString("knob");
        e.label = obj.value("label").toString("Control");
        e.parameter = obj.value("parameter").toString();
        e.x = obj.value("x").toInt(40);
        e.y = obj.value("y").toInt(40);
        e.width = obj.value("width").toInt(90);
        e.height = obj.value("height").toInt(90);
        e.minimum = obj.value("min").toDouble(0.0);
        e.maximum = obj.value("max").toDouble(1.0);
        e.value = obj.value("value").toDouble(0.5);
        e.color = obj.value("color").toString("#61a8ff");
        e.textColor = obj.value("textColor").toString("#edf2f8");
        e.fontSize = obj.value("fontSize").toInt(10);
        return e;
    }
};

struct InstrumentDocument
{
    QString name = "My Instrument";
    int canvasWidth = 800;
    int canvasHeight = 450;
    QString backgroundColor = "#20242b";
    QString backgroundImage;
    QVector<InstrumentElement> elements;

    QJsonObject toJson() const
    {
        QJsonArray array;
        for (const auto &element : elements)
            array.append(element.toJson());

        return {
            {"formatVersion", 1},
            {"instrumentName", name},
            {"canvas", QJsonObject{
                {"width", canvasWidth}, {"height", canvasHeight},
                {"backgroundColor", backgroundColor},
                {"backgroundImage", backgroundImage}
            }},
            {"elements", array}
        };
    }

    static InstrumentDocument fromJson(const QJsonObject &obj)
    {
        InstrumentDocument doc;
        doc.name = obj.value("instrumentName").toString("My Instrument");
        const auto canvas = obj.value("canvas").toObject();
        doc.canvasWidth = canvas.value("width").toInt(800);
        doc.canvasHeight = canvas.value("height").toInt(450);
        doc.backgroundColor = canvas.value("backgroundColor").toString("#20242b");
        doc.backgroundImage = canvas.value("backgroundImage").toString();

        const auto array = obj.value("elements").toArray();
        for (const auto &item : array) {
            if (item.isObject())
                doc.elements.append(InstrumentElement::fromJson(item.toObject()));
        }
        return doc;
    }
};
