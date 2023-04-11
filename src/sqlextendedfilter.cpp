#include "sqlextendedfilter.h"

#include <QDebug>
#include <QFontDatabase>
#include <QLayoutItem>
#include <QSqlRecord>
#include <QStyle>

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
// BEGIN SqlExtendedFilterNode

SqlExtendedFilterNode::SqlExtendedFilterNode(QWidget *parent, QSqlTableModel *m)
    : SqlExtendedFilterNode(parent, m, WithoutInversion, ActionLike)
{
}

SqlExtendedFilterNode::SqlExtendedFilterNode(QWidget *parent, QSqlTableModel *m, SqlExtendedFilterNode::Inversion inv,
                                             SqlExtendedFilterNode::Action act, const QVariant &valueEntered,
                                             const QString &selectedField, const QStringList &hiddenFields)
    : QWidget(parent), model(m), fixed(false)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    setLayout(lay);

    fieldCombo = new QComboBox(this);
    fieldCombo->setMaximumWidth(160);

    fieldCombo->addItem("Поиск по всем столбцам");
    int selectedIdx = 0;
    QSqlRecord rec = model->record();
    for (int i = 0; i < rec.count(); i++)
    {
        QString fieldName = rec.field(i).name();
        if (hiddenFields.contains(fieldName) && selectedField != fieldName)
        {
            continue;
        }

        fieldCombo->addItem(model->headerData(i, Qt::Horizontal).toString(), fieldName);
        if (fieldName == selectedField)
        {
            selectedIdx = fieldCombo->count() - 1;
        }
    }
    fieldCombo->setCurrentIndex(selectedIdx);

    lay->addWidget(fieldCombo);

    inversionCombo = new QComboBox(this);
    inversionCombo->addItem("  ", "");
    inversionCombo->addItem("НЕ", "NOT ");
    inversionCombo->setCurrentIndex(inv);
    lay->addWidget(inversionCombo);

    actionCombo = new QComboBox(this);
    actionCombo->addItem("*T*   Содержит", "LOWER(%1) LIKE '%' || LOWER(%2) || '%'");
    actionCombo->addItem("=     Равно", "%1 = %2");
    actionCombo->addItem(">     Больше", "%1 > %2");
    actionCombo->addItem(">=    Больше или равно", "%1 >= %2");
    actionCombo->addItem("<     Меньше", "%1 < %2");
    actionCombo->addItem("<=    Меньше или равно", "%1 <= %2");
    actionCombo->addItem("[...] В списке", "%1 IN(%2)");
    actionCombo->addItem("T...  Начинается на", "%1 LIKE LOWER(%2) || '%'");
    actionCombo->addItem("...T  Заканчивается на", "%1 LIKE '%' || LOWER(%2)");
    actionCombo->setCurrentIndex(act);
    actionCombo->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    lay->addWidget(actionCombo);

    inputCombo = new QComboBox(this);

    inputCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    inputCombo->setEditable(true);
    inputCombo->setCompleter(nullptr);
    inputCombo->lineEdit()->setPlaceholderText("Введите строку для поиска");

    inputCombo->addItem("");
    inputCombo->addItem(style()->standardIcon(QStyle::SP_DialogCloseButton), "Пустые значения", "NULL");
    inputCombo->installEventFilter(new SqlExtendedFilterUtils::ComboBoxEventFilter(inputCombo));  // Отключаем Enter
    if (valueEntered.isNull())
    {
        inputCombo->setCurrentIndex(1);
    }
    else
    {
        inputCombo->setCurrentText(valueEntered.toString());
    }

    lay->addWidget(inputCombo);

    QPushButton *clearButton = new QPushButton(this);

    clearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    clearButton->setToolTip("Очистить значение");
    lay->addWidget(clearButton);

    QPushButton *deleteButton = new QPushButton("-", this);
    deleteButton->setMaximumWidth(20);
    deleteButton->setToolTip("Удалить правило");
    lay->addWidget(deleteButton);

    QPushButton *addButton = new QPushButton("+", this);
    addButton->setMaximumWidth(20);
    addButton->setToolTip("Добавить правило \"И\"");
    lay->addWidget(addButton);

    connect(addButton, &QPushButton::clicked, this, &SqlExtendedFilterNode::addNodeRequested);
    connect(deleteButton, &QPushButton::clicked, this, &SqlExtendedFilterNode::delNodeRequested);
    connect(clearButton, &QPushButton::clicked, this, &SqlExtendedFilterNode::clearInput);
    connect(inputCombo, &QComboBox::currentTextChanged, this, &SqlExtendedFilterNode::textChanged);
    connect(inputCombo->lineEdit(), &QLineEdit::textEdited, this, &SqlExtendedFilterNode::textEdited);
    connect(actionCombo, &QComboBox::currentTextChanged, this, &SqlExtendedFilterNode::textChanged);
    connect(inversionCombo, &QComboBox::currentTextChanged, this, &SqlExtendedFilterNode::textChanged);
    connect(fieldCombo, &QComboBox::currentTextChanged, this, &SqlExtendedFilterNode::textChanged);
}

QString SqlExtendedFilterNode::text() const
{
    return inputCombo->currentText();
}

QString SqlExtendedFilterNode::textData() const
{
    return inputCombo->currentData().toString();
}

QString SqlExtendedFilterNode::inversion() const
{
    return inversionCombo->currentData().toString();
}

SqlExtendedFilterNode::Inversion SqlExtendedFilterNode::inversion_e() const
{
    return Inversion(inversionCombo->currentIndex());
}

QString SqlExtendedFilterNode::sqlTemplate() const
{
    return actionCombo->currentData().toString();
}

SqlExtendedFilterNode::Action SqlExtendedFilterNode::action_e() const
{
    return Action(actionCombo->currentIndex());
}

QSqlField SqlExtendedFilterNode::field() const
{
    return model->record().field(fieldName());
}

QString SqlExtendedFilterNode::fieldName() const
{
    return fieldCombo->currentData().toString();
}

QString SqlExtendedFilterNode::filter() const
{
    if (fieldName().isEmpty())
    {  // По всем столбцам
        QString output;
        QSqlRecord rec = model->record();
        for (int i = 0; i < fieldCombo->count(); i++)
        {
            QString comboFieldName = fieldCombo->itemData(i).toString();
            if (!comboFieldName.isEmpty())
            {
                QString fieldFilter = filterForField(rec.field(comboFieldName));
                if (!fieldFilter.isEmpty())
                {
                    if (!output.isEmpty())
                    {
                        output += " OR ";
                    }
                    output += fieldFilter;
                }
            }
        }
        return output.isEmpty() ? QString() : QString("(%1)").arg(output);  // Объединить группу в скобки
    }

    return filterForField(field());
}

QString SqlExtendedFilterNode::filterForField(const QSqlField &field) const
{
    if (text().isEmpty())
    {  // Не париться дальше, если поле ввода пустое
        return QString();
    }

    QString sqlTpl = sqlTemplate();
    Action actionType = action_e();
    bool isLike = actionType == ActionLike || actionType == ActionStartsWith || actionType == ActionEndsWith;
    QString inputText = text();
    bool canConvert =
        QVariant(inputText).convert(static_cast<int>(field.type()));  /// \todo canConvert не всегда корректный
    QString expression;
    QString fieldName = QString("`%1`").arg(field.name());

    if (prependTableNameFieldList.isEmpty() || prependTableNameFieldList.contains(field.name()))
    {
        fieldName.prepend(QString("`%1`.").arg(model->tableName()));
    }

    if (textData() == "NULL")
    {  // Если NULL, то никакие другие операторы не сработают, поэтому меняем на IS NULL
        expression = QString("%1 IS NULL").arg(fieldName);
        ;
    }
    else if (!canConvert && actionType != ActionInList)
    {
        return QString();
    }
    else
    {
        if (actionType == ActionInList)
        {
            inputText.replace(QRegExp("\\s*,\\s*"), ",");  // Убрать пробелы вокруг запятых
            QStringList splitted = inputText.split(",", Qt::SkipEmptyParts);

            for (QString &str : splitted)
            {
                str = SqlExtendedFilterUtils::escape(model->database(), field, str);  // Экранировать каждое значение
            }
            inputText = splitted.join(", ");
            expression = sqlTpl.arg(fieldName, inputText);
        }
        else
        {
            inputText = SqlExtendedFilterUtils::escape(model->database(), field, inputText);
            expression = sqlTpl.arg(fieldName, inputText);
            if (isLike && field.type() != QVariant::String)
            {
                // LIKE работет только для строк
                expression = QString("%1 = %2").arg(fieldName, inputText);
            }
        }
    }

    return QString("(%1(%2))").arg(inversion(), expression);
}

void SqlExtendedFilterNode::clearInput()
{
    inputCombo->setCurrentIndex(-1);
    inputCombo->setCurrentText("");
}

void SqlExtendedFilterNode::textChanged(const QString &)
{
    actionCombo->setEnabled(textData().isEmpty());
    emit nodeDataChanged();
}

void SqlExtendedFilterNode::textEdited(const QString &txt)
{
    if (!textData().isEmpty())
    {  // Запрещаем редактирование ранее заданных значений
        inputCombo->setCurrentIndex(-1);
        inputCombo->setCurrentText(txt);
    }
}

void SqlExtendedFilterNode::setPrependTableNameFieldList(const QStringList &value)
{
    prependTableNameFieldList = value;
}

bool SqlExtendedFilterNode::isFixed() const
{
    return fixed;
}

void SqlExtendedFilterNode::setFixed(bool value)
{
    setEnabled(!value);
    fixed = value;
}

// END SqlExtendedFilterNode
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
// BEGIN SqlExtendedFilter

SqlExtendedFilter::SqlExtendedFilter(QWidget *parent) : QWidget(parent), model(nullptr)
{
    lay = new QVBoxLayout;
    lay->setMargin(0);
    setLayout(lay);
    input = nullptr;
    fieldSelector = nullptr;
}

void SqlExtendedFilter::addDefaultRule()
{
    addRule(SqlExtendedFilterNode::WithoutInversion, SqlExtendedFilterNode::ActionLike);
}

void SqlExtendedFilter::addRule(SqlExtendedFilterNode::Inversion inv, SqlExtendedFilterNode::Action act,
                                const QVariant &val, const QSqlField &f, bool fixed)
{
    if (model)
    {
        SqlExtendedFilterNode *node = new SqlExtendedFilterNode(this, model, inv, act, val, f.name(), hiddenFields);
        node->setFixed(fixed);
        node->setPrependTableNameFieldList(prependTableNameTo);

        connect(node, &SqlExtendedFilterNode::addNodeRequested, this, &SqlExtendedFilter::addDefaultRule);
        connect(node, &SqlExtendedFilterNode::delNodeRequested, this, &SqlExtendedFilter::delNode);
        connect(node, &SqlExtendedFilterNode::nodeDataChanged, this, &SqlExtendedFilter::nodeFilterChanged);

        if (SqlExtendedFilterNode *n = qobject_cast<SqlExtendedFilterNode *>(sender()))
        {
            int idx = lay->indexOf(n);
            if (idx > -1)
            {
                lay->insertWidget(idx + 1, node);
                return;
            }
        }
        lay->addWidget(node);

        if (val.isNull() || !val.toString().isEmpty())
        {
            nodeFilterChanged();
        }
    }
}

void SqlExtendedFilter::delNode()
{
    if (SqlExtendedFilterNode *node = qobject_cast<SqlExtendedFilterNode *>(sender()))
    {
        int idx = lay->indexOf(node);
        if (idx > -1)
        {
            node->deleteLater();
            delete lay->takeAt(idx);
        }
        if (lay->count() < 1)
        {
            addDefaultRule();
        }
    }
    nodeFilterChanged();
}

void SqlExtendedFilter::clear()
{
    for (int i = lay->count() - 1; i > -1; i--)
    {
        if (SqlExtendedFilterNode *node = qobject_cast<SqlExtendedFilterNode *>(lay->itemAt(i)->widget()))
        {
            if (!node->isFixed())
            {
                node->deleteLater();
                delete lay->takeAt(i);
            }
        }
    }
    nodeFilterChanged();
}

QString SqlExtendedFilter::filter() const
{
    QString total;
    for (int i = 0; i < lay->count(); i++)
    {
        if (const SqlExtendedFilterNode *n_node = node(i))
        {
            QString f = n_node->filter();
            if (!f.isEmpty())
            {
                if (!total.isEmpty())
                {
                    total += " AND ";
                }
                total += f;
            }
        }
    }
    return total;
}

int SqlExtendedFilter::count()
{
    return lay->count();
}

const SqlExtendedFilterNode *SqlExtendedFilter::node(int n) const
{
    if (n < 0 || n >= lay->count())
    {
        return nullptr;
    }
    return qobject_cast<SqlExtendedFilterNode *>(lay->itemAt(n)->widget());
}

void SqlExtendedFilter::nodeFilterChanged()
{
    emit filterChanged(filter());
}

void SqlExtendedFilter::setHiddenFields(const QStringList &value)
{
    hiddenFields = value;
    clear();
    addDefaultRule();
}

void SqlExtendedFilter::setFieldsByHeader(QHeaderView *header)
{
    QSqlRecord rec = model->record();
    QStringList fields;
    for (int idx = 0; idx < header->count(); idx++)
    {
        if (header->isSectionHidden(idx))
        {
            fields << rec.fieldName(idx);
        }
    }
    setHiddenFields(fields);
}

void SqlExtendedFilter::setPrependTableNameTo(const QStringList &fields)
{
    prependTableNameTo = fields;
}

void SqlExtendedFilter::setModel(QSqlTableModel *value)
{
    clear();
    model = value;
    addDefaultRule();
}

// END SqlExtendedFilter
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
// BEGIN ComboBoxEventFilter

SqlExtendedFilterUtils::ComboBoxEventFilter::ComboBoxEventFilter(QObject *parent) : QObject(parent)
{
}

bool SqlExtendedFilterUtils::ComboBoxEventFilter::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            return true;
        }
    }
    return QObject::eventFilter(object, event);
}

// END ComboBoxEventFilter
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
// BEGIN SqlExtendedFilterUtils

QString SqlExtendedFilterUtils::escape(const QSqlDatabase &db, QSqlField f, const QVariant &value)
{
    f.setValue(value);
    return db.driver()->formatValue(f);
}

// END SqlExtendedFilterUtils
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
