#ifndef COMPOSER_H
#define COMPOSER_H

#include <QObject>
#include <QVariant>

class QIODevice;

namespace QuComposer
{

enum PropertySetterResult
{
	PropertySetterSucceeded,
	PropertySetterFailed,
	PropertySetterIgnored
};

typedef PropertySetterResult (*PropertySetter)(QWidget *widget, const QString &propertyName, const QVariant &data);

class Composer : public QObject
{
	Q_OBJECT
	
public:
	
	explicit Composer(QObject *parent = 0);
	
	void addPropertySetter(PropertySetter setter) { _setters << setter; }
	
	void parse(QIODevice *device, bool *ok = 0);
	
	QWidget *composeWidget();
	
signals:
	
public slots:
	
private:
	
	QWidget *composeWidgetRecursive(const QVariantMap &classData);
	bool setWidgetProperty(QWidget *widget, const QString &propertyName, const QVariant &data);
	QWidget *createWidget(const QString &className);
	
	QVariant convertPropertyData(const QVariant &data, const QMetaProperty &metaProperty);
	
	QVariant _data;
	QList<PropertySetter> _setters;
};

}

#endif // COMPOSER_H
