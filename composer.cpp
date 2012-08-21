#include <QtGui>
#include "qjson/include/QJson/Parser"

#include "composer.h"

namespace QuComposer
{

QPoint listToPoint(const QVariantList &list)
{
	return QPoint(list.at(0).toInt(), list.at(1).toInt());
}

QSize listToSize(const QVariantList &list)
{
	return QSize(list.at(0).toInt(), list.at(1).toInt());
}

QRect listToRect(const QVariantList &list)
{
	return QRect(list.at(0).toInt(), list.at(1).toInt(), list.at(2).toInt(), list.at(3).toInt());
}

QMargins listToMargins(const QVariantList &list)
{
	return QMargins(list.at(0).toInt(), list.at(1).toInt(), list.at(2).toInt(), list.at(3).toInt());
}

QSizePolicy::Policy stringToPolicy(const QString &string)
{
	if (string == "Fixed")
		return QSizePolicy::Fixed;
	if (string == "Minimum")
		return QSizePolicy::Minimum;
	if (string == "Maximum")
		return QSizePolicy::Maximum;
	if (string == "Preferred")
		return QSizePolicy::Preferred;
	if (string == "Expanding")
		return QSizePolicy::Expanding;
	if (string == "MinimumExpanding")
		return QSizePolicy::MinimumExpanding;
	if (string == "Ignored")
		return QSizePolicy::Ignored;
	else
		return QSizePolicy::Fixed;
}

QSizePolicy listToSizePolicy(const QVariantList &list)
{
	return QSizePolicy(stringToPolicy(list.at(0).toString()), stringToPolicy(list.at(1).toString()));
}

Composer::Composer(QObject *parent) :
    QObject(parent)
{
}

void Composer::parse(QIODevice *device, bool *okout)
{
	QJson::Parser parser;
	bool ok;
	_data = parser.parse(device, &ok);
	if (ok == false)
		qWarning() << "parse error";
	
	if (okout)
		*okout = ok;
}

QWidget *Composer::composeWidget()
{
	return composeWidgetRecursive(_data.toMap());
}

QWidget *Composer::composeWidgetRecursive(const QVariantMap &classData)
{
	QString className = classData["class"].toString();
	
	qDebug() << "class name:" << className;
	
	if (className.isEmpty())
	{
		qDebug() << "no class name found.";
		return 0;
	}
	
	QWidget *widget = createWidget(className);
	
	if (widget == 0)
	{
		qDebug() << "the class is not supported.";
		return 0;
	}
	
	{
		QVariantMap properties = classData["properties"].toMap();
		QVariantMap::const_iterator iter;
		
		for (iter = properties.constBegin(); iter != properties.constEnd(); ++iter)
		{
			setWidgetProperty(widget, iter.key().toAscii(), iter.value());
		}
	}
	
	{
		QVariantList children = classData["children"].toList();
		QVariantList::const_iterator iter;
		
		for (iter = children.constBegin(); iter != children.constEnd(); ++iter)
		{
			QWidget *child = composeWidgetRecursive(iter->toMap());
			child->setParent(widget);
		}
	}
	
	return widget;
}

bool Composer::setWidgetProperty(QWidget *widget, const QString &propertyName, const QVariant &data)
{
	const QMetaObject *metaObj = widget->metaObject();
	
	int propertyIndex = metaObj->indexOfProperty(propertyName.toAscii());
	
	if (propertyIndex < 0)
		return false;
	
	foreach (PropertySetter setter, _setters)
	{
		PropertySetterResult result = setter(widget, propertyName, data);
		
		if (result == PropertySetterSucceeded)
			return true;
		if (result == PropertySetterFailed)
			return false;
	}
	
	QMetaProperty metaProperty = metaObj->property(propertyIndex);
	
	if (metaProperty.isWritable() == false)
		return false;
	
	QVariant v = convertPropertyData(data, metaProperty);
	return v.isValid() ? widget->setProperty(propertyName.toAscii(), v) : false;
}

QVariant Composer::convertPropertyData(const QVariant &data, const QMetaProperty &metaProperty)
{
	QVariant::Type type = metaProperty.type();
	
	if (data.canConvert(type))
	{
		QVariant r = data;
		r.convert(type);
		return r;
	}
	
	switch (type)
	{
		case QVariant::Point:
			return listToPoint(data.toList());
		case QVariant::Size:
			return listToSize(data.toList());
		case QVariant::Rect:
			return listToRect(data.toList());
		default:
			break;
	}
	
	QString typeName(metaProperty.typeName());
	
	if (typeName == "QSizePolicy")
	{
		return QVariant::fromValue(listToSizePolicy(data.toList()));
	}
	
	return QVariant();
}

QWidget *Composer::createWidget(const QString &className)
{
	if (className == "QWidget")
		return new QWidget();
	
	return 0;
}

}
