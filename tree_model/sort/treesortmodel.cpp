#include "treesortmodel.h"
#include "../treeitemmodel.h"

using namespace arcirk::tree_model;

TreeSortModel::TreeSortModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    setProperty("typeName", "TreeSortModel");
    this->setDynamicSortFilter(true);
}

void TreeSortModel::reset()
{
    beginResetModel();
    endResetModel();
}

void TreeSortModel::addFilter(const QString &column_name, const QVariant &value, type_of_comparison compare)
{
    if(sourceModel()){
        beginResetModel();
    }
    m_filter.insert(column_name, qMakePair(compare, value));
    if(sourceModel()){
        endResetModel();
    }
}

void TreeSortModel::setFilter(const QString &filterText, int compare)
{
    m_filter.clear();

    if(!json::accept(filterText.toStdString()))
        return;

    json object = json::parse(filterText.toStdString());

    for (auto itr = object.items().begin(); itr != object.items().end(); ++itr) {
        auto val = to_variant(itr.value());
        addFilter(itr.key().c_str(), val, (type_of_comparison)compare);
    }
}

void TreeSortModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
    emit modelChanged();
}

QVariant TreeSortModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(sourceModel() && role == Qt::InitialSortOrderRole){
        auto model = (TreeItemModel*)sourceModel();
        if(section == model->column_index("is_group") && model->hierarchical_list()){
            return Qt::DescendingOrder;
        }else if(section == model->column_index("first") && model->hierarchical_list()){
            return Qt::AscendingOrder;
        }
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

QString TreeSortModel::dump(const int &row) const
{
    auto model = (TreeItemModel*)sourceModel();
    if(model){
        auto index = mapToSource(this->index(row, 0));
        return model->to_object(index).dump().c_str();
    }
    return "";
}

bool TreeSortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {

    auto model = (TreeItemModel*)sourceModel();

    if (m_filter.size() > 0){
        QMapIterator<QString, QPair<type_of_comparison,QVariant>> i(m_filter);
        while (i.hasNext()){
            i.next();
            int col = model->column_index(i.key());
            if(col == -1)
                continue;
            QModelIndex index = model->index(sourceRow, col, sourceParent);

            auto compare = i.value().first;
            auto source = i.value().second;
            auto value = model->data(index, Qt::DisplayRole);
            return isCompareValid(compare, value, source);
        }

    }

    return true;
}

bool TreeSortModel::isCompareValid(type_of_comparison compare, const QVariant &value, const QVariant &source) const
{
    bool result = false;
    switch (compare) {
    case Equals:
        result = value == source;
        break;
    case Not_Equals:
        result = value != source;
        break;
    case More:
        if(value.typeId() == QMetaType::Double)
            result = value.toDouble() > source.toDouble();
        else if(value.typeId() == QMetaType::Int){
            result = value.toInt() > source.toInt();
        }
        break;
    case Less:
        if(value.typeId() == QMetaType::Double)
            result = value.toDouble() < source.toDouble();
        else if(value.typeId() == QMetaType::Int){
            result = value.toInt() < source.toInt();
        }
        break;
    case More_Or_Equal:
        if(value.typeId() == QMetaType::Double)
            result = value.toDouble() >= source.toDouble();
        else if(value.typeId() == QMetaType::Int){
            result = value.toInt() >= source.toInt();
        }
        break;
    case Less_Or_Equal:
        if(value.typeId() == QMetaType::Double)
            result = value.toDouble() <= source.toDouble();
        else if(value.typeId() == QMetaType::Int){
            result = value.toInt() <= source.toInt();
        }
        break;
    case On_List:
        result = source.toStringList().indexOf(value.toString()) != -1;
        break;
    case Not_In_List:
        result = source.toStringList().indexOf(value.toString()) == -1;
        break;
    case Contains:{
            auto val = value.toString();
            result = val.indexOf(source.toString()) != -1;
        }
        break;
    default:
        break;
    }

    return result;
}
