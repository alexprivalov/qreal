/* Copyright 2007-2015 QReal Research Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "propertyEditorModel.h"
#include "details/logicalModel.h"

#include <qrkernel/exception/exception.h>
#include <qrkernel/definitions.h>

using namespace qReal;

PropertyEditorModel::PropertyEditorModel(
		const qReal::EditorManagerInterface &editorManagerInterface
		, QObject *parent
		)
	: QAbstractTableModel(parent)
	, mTargetLogicalModel(nullptr)
	, mTargetGraphicalModel(nullptr)
	, mEditorManagerInterface(editorManagerInterface)
{
}

int PropertyEditorModel::rowCount(const QModelIndex&) const
{
	return mFields.size();
}

int PropertyEditorModel::columnCount(const QModelIndex&) const
{
	return 2;
}

Qt::ItemFlags PropertyEditorModel::flags(const QModelIndex &index) const
{
	// Property names
	if (index.column() == 0)
		return Qt::ItemIsEnabled;

	switch (mFields[index.row()].attributeClass) {
	case logicalAttribute:
	case graphicalAttribute:
	case namePseudoattribute:
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	case graphicalIdPseudoattribute:
	case logicalIdPseudoattribute:
	case metatypePseudoattribute:
	default:
		return Qt::NoItemFlags;
	}
}

QVariant PropertyEditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
		return QString(section == 1 ? "value" : "name");
	else
		return QVariant();
}

QVariant PropertyEditorModel::data(const QModelIndex &index, int role) const
{
	if (!isValid()) {
		return QVariant();
	}

	if (role == Qt::ToolTipRole) {
		if (index.column() == 0) {
			const Id id = mTargetLogicalObject.data(roles::idRole).value<Id>();
			const QString description = mEditorManagerInterface.propertyDescription(id, mFields[index.row()].fieldName);
			if (!description.isEmpty()) {
				return "<body>" + description;
			} else {
				return QVariant();
			}
		} else if (index.column() == 1) {
			return data(index, Qt::DisplayRole);
		} else {
			return QVariant();
		}
	}

	if (role != Qt::DisplayRole) {
		return QVariant();
	}

	if (index.column() == 0) {
		const Id id = mTargetLogicalObject.data(roles::idRole).value<Id>();
		const QString displayedName = mEditorManagerInterface.propertyDisplayedName(id, mFields[index.row()].fieldName);
		return displayedName.isEmpty() ? mFields[index.row()].fieldName : displayedName;
	} else if (index.column() == 1) {
		switch (mFields[index.row()].attributeClass) {
		case logicalAttribute: {
			return mTargetLogicalObject.data(mFields[index.row()].role).toString();
		}
		case graphicalAttribute:
			return mTargetGraphicalObject.data(mFields[index.row()].role);
		case graphicalIdPseudoattribute:
			return mTargetGraphicalObject.data(roles::idRole).value<Id>().id();
		case logicalIdPseudoattribute:
			return mTargetLogicalObject.data(roles::idRole).value<Id>().id();
		case metatypePseudoattribute: {
			const Id id = mTargetLogicalObject.data(roles::idRole).value<Id>();
			return QVariant(id.editor() + "/" + id.diagram() + "/" + id.element());
		}
		case namePseudoattribute: {
			return mTargetLogicalObject.data(Qt::DisplayRole);
		}
		default:
			return QVariant();
		}
	} else {
		return QVariant();
	}
}

bool PropertyEditorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	bool modelChanged = true;

	if (!isValid()) {
		return false;
	}

	if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.column() == 1) {
		switch (mFields[index.row()].attributeClass) {
		case logicalAttribute: {
			mTargetLogicalModel->setData(mTargetLogicalObject, value, mFields[index.row()].role);
			break;
		}
		case graphicalAttribute:
			mTargetGraphicalModel->setData(mTargetGraphicalObject, value, mFields[index.row()].role);
			break;
		case namePseudoattribute:
			mTargetLogicalModel->setData(mTargetLogicalObject, value, Qt::DisplayRole);
			break;
		default:
			modelChanged = false;
		}
	} else {
		modelChanged = false;
	}

	if (modelChanged) {
		emit dataChanged(index, index);
	}

	return modelChanged;
}

bool PropertyEditorModel::enumEditable(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return false;
	}

	const AttributeClassEnum attrClass = mFields[index.row()].attributeClass;
	// metatype, ids and name are definitely not enums
	if (attrClass != logicalAttribute && attrClass != graphicalAttribute) {
		return false;
	}

	const Id id = attrClass == logicalAttribute
			? mTargetLogicalObject.data(roles::idRole).value<Id>()
			: mTargetGraphicalObject.data(roles::idRole).value<Id>();

	return mEditorManagerInterface.isEnumEditable(id, mFields[index.row()].fieldName);
}

QList<QPair<QString, QString>> PropertyEditorModel::enumValues(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return {};
	}

	const AttributeClassEnum attrClass = mFields[index.row()].attributeClass;
	// metatype, ids and name are definitely not enums
	if (attrClass != logicalAttribute && attrClass != graphicalAttribute) {
		return {};
	}

	const Id id = attrClass == logicalAttribute
			? mTargetLogicalObject.data(roles::idRole).value<Id>()
			: mTargetGraphicalObject.data(roles::idRole).value<Id>();

	/// @todo: null id must not be met here but for some reason sometimes it happens.
	/// This is pretty strange because mTargetLogicalObject without manual modification
	/// becomes invalid index.
	return id.isNull()
			? QList<QPair<QString, QString>>()
			: mEditorManagerInterface.enumValues(id, mFields[index.row()].fieldName);
}

void PropertyEditorModel::rereadData(const QModelIndex &topLeftIndex, const QModelIndex &bottomRightIndex)
{
	emit dataChanged(topLeftIndex, bottomRightIndex);
}

void PropertyEditorModel::setSourceModels(QAbstractItemModel * const sourceLogicalModel
		, QAbstractItemModel * const sourceGraphicalModel)
{
	mTargetLogicalModel = sourceLogicalModel;
	mTargetGraphicalModel = sourceGraphicalModel;

	beginResetModel();
	mFields.clear();
	endResetModel();

	if (mTargetLogicalModel)
		connect(mTargetLogicalModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &))
				, this, SLOT(rereadData(const QModelIndex &, const QModelIndex &)));

	// At the moment property editor does not show graphical properties at all.
	// If this should happen then dataChanged() signal of graphical model should be connected here too.
	// WARNING: This should not be done before rewriting property editor model completely.
	// Connecting graphical model here will drop QReal performance dramatically. This was checked on sad experience.
	// For each modification in graphical model setting arbitrary property (position of node, configuration of edge)
	// will cause full properties list rereading.
}

void PropertyEditorModel::setModelIndexes(const QModelIndex &logicalModelIndex
		, const QModelIndex &graphicalModelIndex)
{
	beginResetModel();
	mFields.clear();
	endResetModel();

	mTargetLogicalObject = logicalModelIndex;
	mTargetGraphicalObject = graphicalModelIndex;

	if (!isValid()) {
		return;
	}

	const Id logicalId = mTargetLogicalObject.data(roles::idRole).value<Id>();
	const QString dynamicProperties = dynamic_cast<models::details::LogicalModel *>(mTargetLogicalModel)->
			logicalModelAssistApi().logicalRepoApi().stringProperty(logicalId, "dynamicProperties");

	if (logicalModelIndex != QModelIndex()) {
		const QStringList logicalProperties = mEditorManagerInterface.propertyNames(logicalId.type());
		int role = roles::customPropertiesBeginRole;
		foreach (QString property, logicalProperties) {
			mFields << Field(property, logicalAttribute, role);
			++role;
		}

		if (!dynamicProperties.isEmpty()) {
			QDomDocument dynamProperties;
			dynamProperties.setContent(dynamicProperties);
			for (QDomElement element = dynamProperties.firstChildElement("properties").firstChildElement("property")
					; !element.isNull()
					; element = element.nextSiblingElement("property"))
			{
				mFields << Field(element.attribute("text"), logicalAttribute, role);
				++role;
			}
		}
	}

	beginResetModel();
	endResetModel();
}

void PropertyEditorModel::clearModelIndexes()
{
	setModelIndexes(QModelIndex(), QModelIndex());
}

bool PropertyEditorModel::isCurrentIndex(const QModelIndex &index) const
{
	return index == mTargetLogicalObject || index == mTargetGraphicalObject;
}

bool PropertyEditorModel::isValid() const
{
	return mTargetGraphicalModel && mTargetLogicalModel
			&& (mTargetLogicalObject.isValid() || mTargetGraphicalObject.isValid());
}

QModelIndex PropertyEditorModel::modelIndex(int row) const
{
	switch (mFields[row].attributeClass) {
	case logicalAttribute:
		return mTargetLogicalObject;
	case graphicalAttribute:
		return mTargetGraphicalObject;
	default:
		throw Exception("PropertyEditorModel::modelIndex: called for incorrect field,"
				"which is not graphical nor logical attribute");
	}
	return QModelIndex();
}

int PropertyEditorModel::roleByIndex(int row) const
{
	return mFields[row].role;
}

QString PropertyEditorModel::typeName(const QModelIndex &index) const
{
	Id id = idByIndex(index);
	if (id.isNull()) {
		return "";
	}
	return mEditorManagerInterface.typeName(id, mFields[index.row()].fieldName);
}

QString PropertyEditorModel::propertyName(const QModelIndex &index) const
{
	QString fieldName = mFields[index.row()].fieldName;
	const Id logicalId = mTargetLogicalObject.data(roles::idRole).value<Id>();
	const QString dynamicProperties = dynamic_cast<models::details::LogicalModel *>(mTargetLogicalModel)->
	logicalModelAssistApi().logicalRepoApi().stringProperty(logicalId, "dynamicProperties");

	if (!dynamicProperties.isEmpty()) {
		int propertiesCount = mEditorManagerInterface.propertyNames(logicalId.type()).count();
		QDomDocument dynamProperties;
		dynamProperties.setContent(dynamicProperties);
		int i = 0;
		for (QDomElement element
				= dynamProperties.firstChildElement("properties").firstChildElement("property");
				!element.isNull();
				element = element.nextSiblingElement("property"))
		{
			if (i == index.row() - propertiesCount) {
				fieldName = element.attribute("textBinded");
				break;
			}
			++i;
		}
	}

	return fieldName;
}

bool PropertyEditorModel::setData(const Id &id, const QString &propertyName, const QVariant &value)
{
	if (mFields.isEmpty() || idByIndex(index(0, 0)) != id) {
		return false;
	}

	for (const Field &field : mFields) {
		if (field.fieldName == propertyName) {
			setData(index(field.role, 1), value);
			return true;
		}
	}

	return false;
}

bool PropertyEditorModel::isReference(const QModelIndex &index, const QString &propertyName)
{
	Id id = idByIndex(index);
	if (id.isNull()) {
		return false;
	}
	return mEditorManagerInterface.referenceProperties(id.type()).contains(propertyName);
}

Id PropertyEditorModel::idByIndex(const QModelIndex &index) const
{
	switch (mFields[index.row()].attributeClass) {
		case logicalAttribute:
			return mTargetLogicalObject.data(roles::idRole).value<Id>();
		case graphicalAttribute:
			return mTargetGraphicalObject.data(roles::idRole).value<Id>();
		default:
			return Id();
	}
}
