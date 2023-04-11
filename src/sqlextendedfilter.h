#ifndef SQLEXTENDEDFILTER_HPP
#define SQLEXTENDEDFILTER_HPP

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSqlTableModel>
#include <QDialog>
#include <QSqlRecord>
#include <QSpacerItem>
#include <QGroupBox>
#include <QSqlField>
#include <QDebug>
#include <QKeyEvent>
#include <QSqlDriver>
#include <QBoxLayout>
#include <QHeaderView>

namespace SqlExtendedFilterUtils {

/// Отключение кнопки Enter для QComboBox
class ComboBoxEventFilter : public QObject {
    Q_OBJECT
public:
    ComboBoxEventFilter(QObject *parent = nullptr);
protected:
    bool eventFilter(QObject *object, QEvent *event);
};

QString escape(const QSqlDatabase &db, QSqlField f, const QVariant &value); ///< Экранирование данных для базы данных

};

///< Виджет-правило для SqlExtendedFilter
class SqlExtendedFilterNode : public QWidget {
    Q_OBJECT
public:

    /// Индекс SqlExtendedFilterNode::actionCombo
    enum Action {
        ActionLike, ///< Содержит
        ActionEquals, ///< Равно
        ActionMoreThan, ///< Больше
        ActionMoreThanInc, ///< Больше или равно
        ActionLessThan, ///< Меньше
        ActionLessThanInc, ///< Меньше или равно
        ActionInList, ///< В списке
        ActionStartsWith, ///< Начинается на
        ActionEndsWith, ///< Заканчивается на
    };

    /// Индекс SqlExtendedFilterNode::inversionCombo
    enum Inversion {
        WithoutInversion, ///< пусто
        WithInversion ///< НЕ
    };

    explicit SqlExtendedFilterNode(QWidget *parent, QSqlTableModel *m);
    explicit SqlExtendedFilterNode(QWidget *parent,
                                   QSqlTableModel *m,
                                   Inversion inv,
                                   Action act,
                                   const QVariant &valueEntered = QString(""),
                                   const QString &selectedField = QString(),
                                   const QStringList &hiddenFields = QStringList());

    QString text() const; ///< Текст поля ввода
    QString textData() const; ///< Данные поля ввода (для обработки пункта "Пустые значения"
    QString inversion() const; ///< Получение параметра инверсии данных ("NOT или пусто")
    Inversion inversion_e() const; ///< Получение параметра инверсии данных
    QString sqlTemplate() const; ///< Получение sql шаблона ("%1 LIKE %2", "%1 = %2" и других)
    Action action_e() const; ///< Получение типа действия
    QSqlField field() const; ///< Получение поля таблицы
    QString fieldName() const; ///< Получение имени поля таблицы (системное, не пользовательское)
    QString filter() const; ///< Создание и возврат фильтра
    QString filterForField(const QSqlField &field) const; ///< Создание и возврат фильтра для конкретного поля

    bool isFixed() const;
    void setFixed(bool value); ///< Заблокировать изменение и удаление правила

    void setPrependTableNameFieldList(const QStringList &value);

signals:

    void addNodeRequested(); ///< При нажатии кнопки +
    void delNodeRequested(); ///< При нажатии кнопки -
    void nodeDataChanged(); ///< При изменении данных

private slots:
    void clearInput(); ///< Очистка поля ввода
    void textChanged(const QString &); ///< Текст поля ввода изменён программой или пользователем
    void textEdited(const QString &txt);  ///< Текст поля ввода изменён пользователем

private:
    QSqlTableModel *model;
    QComboBox *inputCombo;
    QComboBox *fieldCombo;
    QComboBox *actionCombo;
    QComboBox *inversionCombo;
    bool fixed;
    QStringList prependTableNameFieldList;
};


class SqlExtendedFilter : public QWidget {
    Q_OBJECT
public:
    explicit SqlExtendedFilter(QWidget *parent = nullptr);
    void setModel(QSqlTableModel *value); ///< Установка модели, обязательно
    void clear(); ///< Удаление всех правил

    QString filter() const; ///< Создание и возврат фильтра
    int count(); ///< Количество правил
    const SqlExtendedFilterNode *node(int n) const; ///< Получить правило

    void addRule(SqlExtendedFilterNode::Inversion inv,
                 SqlExtendedFilterNode::Action act, const QVariant &val = QString(""), const
                 QSqlField &f = QSqlField(),
                 bool fixed = false); ///< Добавление правила

    void addPrependTableNameTo(const QString &field);

    void setPrependTableNameTo(const QStringList &fields);

    void setHiddenFields(const QStringList &value);

    void setFieldsByHeader(QHeaderView *header);

signals:
    void filterChanged(const QString &filter); ///< Вызывается при изменении правил, а также при их удалении

public slots:
    void addDefaultRule(); ///< Добавление правила

private slots:
    void delNode(); ///< Удаление правила
    void nodeFilterChanged(); ///< Обработчик изменения правила
private:
    QVBoxLayout *lay;
    QLineEdit *input;
    QComboBox *fieldSelector;
    QSqlTableModel *model;
    QStringList prependTableNameTo;
    QStringList hiddenFields;
};




#endif /* SQLEXTENDEDFILTER_HPP */
