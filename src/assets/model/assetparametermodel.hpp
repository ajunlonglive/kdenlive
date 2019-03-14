/***************************************************************************
 *   Copyright (C) 2017 by Nicolas Carion                                  *
 *   This file is part of Kdenlive. See www.kdenlive.org.                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) version 3 or any later version accepted by the       *
 *   membership of KDE e.V. (or its successor approved  by the membership  *
 *   of KDE e.V.), which shall act as a proxy defined in Section 14 of     *
 *   version 3 of the license.                                             *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef ASSETPARAMETERMODEL_H
#define ASSETPARAMETERMODEL_H

#include "definitions.h"
#include "klocalizedstring.h"
#include <QAbstractListModel>
#include <QDomElement>
#include <QJsonDocument>
#include <QUndoCommand>
#include <unordered_map>

#include <memory>
#include <mlt++/MltProperties.h>

class KeyframeModelList;
/* @brief This class is the model for a list of parameters.
   The behaviour of a transition or an effect is typically  controlled by several parameters. This class exposes this parameters as a list that can be rendered
   using the relevant widgets.
   Note that internally parameters are not sorted in any ways, because some effects like sox need a precise order

 */

enum class ParamType {
    Double,
    List,
    Bool,
    Switch,
    RestrictedAnim, // animated 1 dimensional param with linear support only
    Animated,
    AnimatedRect,
    Geometry,
    Addedgeometry,
    KeyframeParam,
    Color,
    ColorWheel,
    Position,
    Curve,
    Bezier_spline,
    Roto_spline,
    Wipe,
    Url,
    Keywords,
    Fontfamily,
    Filterjob,
    Readonly,
    Hidden
};
Q_DECLARE_METATYPE(ParamType)
class AssetParameterModel : public QAbstractListModel, public enable_shared_from_this_virtual<AssetParameterModel>
{
    Q_OBJECT

public:
    explicit AssetParameterModel(std::unique_ptr<Mlt::Properties> asset, const QDomElement &assetXml, const QString &assetId, ObjectId ownerId,
                                 QObject *parent = nullptr);
    ~AssetParameterModel() override;
    enum DataRoles {
        NameRole = Qt::UserRole + 1,
        TypeRole,
        CommentRole,
        MinRole,
        MaxRole,
        DefaultRole,
        SuffixRole,
        DecimalsRole,
        ValueRole,
        AlphaRole,
        ListValuesRole,
        ListNamesRole,
        FactorRole,
        FilterRole,
        ScaleRole,
        OpacityRole,
        RelativePosRole,
        // Don't display this param in timeline keyframes
        ShowInTimelineRole,
        InRole,
        OutRole,
        ParentInRole,
        ParentPositionRole,
        ParentDurationRole,
        HideKeyframesFirstRole,
        List1Role,
        List2Role,
        Enum1Role,
        Enum2Role,
        Enum3Role,
        Enum4Role,
        Enum5Role,
        Enum6Role,
        Enum7Role,
        Enum8Role,
        Enum9Role,
        Enum10Role,
        Enum11Role,
        Enum12Role,
        Enum13Role,
        Enum14Role,
        Enum15Role,
        Enum16Role
    };

    /* @brief Returns the id of the asset represented by this object */
    QString getAssetId() const;

    /* @brief Set the parameter with given name to the given value
     */
    Q_INVOKABLE void setParameter(const QString &name, const QString &paramValue, bool update = true, const QModelIndex &paramIndex = QModelIndex());
    void setParameter(const QString &name, const int value, bool update = true);
    Q_INVOKABLE void setParameter(const QString &name, double &value);

    /* @brief Return all the parameters as pairs (parameter name, parameter value) */
    QVector<QPair<QString, QVariant>> getAllParameters() const;
    /* @brief Returns a json definition of the effect with all param values */
    QJsonDocument toJson() const;
    void savePreset(const QString &presetFile, const QString &presetName);
    void deletePreset(const QString &presetFile, const QString &presetName);
    const QStringList getPresetList(const QString &presetFile) const;
    const QVector<QPair<QString, QVariant>> loadPreset(const QString &presetFile, const QString &presetName);

    /* @brief Sets the value of a list of parameters
       @param params contains the pairs (parameter name, parameter value)
     */
    void setParameters(const QVector<QPair<QString, QVariant>> &params);

    /* Which monitor is attached to this asset (clip/project)
     */
    Kdenlive::MonitorId monitorId;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /* @brief Returns the id of the actual object associated with this asset */
    ObjectId getOwnerId() const;

    /* @brief Returns the keyframe model associated with this asset
       Return empty ptr if there is no keyframable parameter in the asset or if prepareKeyframes was not called
     */
    Q_INVOKABLE std::shared_ptr<KeyframeModelList> getKeyframeModel();

    /* @brief Must be called before using the keyframes of this model */
    void prepareKeyframes();
    void resetAsset(std::unique_ptr<Mlt::Properties> asset);
    /* @brief Returns true if the effect has more than one keyframe */
    bool hasMoreThanOneKeyframe() const;
    int time_to_frames(const QString &time);
    void passProperties(Mlt::Properties &target);

protected:
    /* @brief Helper function to retrieve the type of a parameter given the string corresponding to it*/
    static ParamType paramTypeFromStr(const QString &type);

    static QString getDefaultKeyframes(int start, const QString &defaultValue, bool linearOnly);

    /* @brief Helper function to get an attribute from a dom element, given its name.
       The function additionally parses following keywords:
       - %width and %height that are replaced with profile's height and width.
       If keywords are found, mathematical operations are supported for double type params. For example "%width -1" is a valid value.
    */
    static QVariant parseAttribute(const ObjectId owner, const QString &attribute, const QDomElement &element, QVariant defaultValue = QVariant());

    /* @brief Helper function to register one more parameter that is keyframable.
       @param index is the index corresponding to this parameter
    */
    void addKeyframeParam(const QModelIndex index);

    struct ParamRow
    {
        ParamType type;
        QDomElement xml;
        QVariant value;
        QString name;
    };

    QString m_assetId;
    ObjectId m_ownerId;
    std::vector<QString> m_paramOrder;                   // Keep track of parameter order, important for sox
    std::unordered_map<QString, ParamRow> m_params;      // Store all parameters by name
    std::unordered_map<QString, QVariant> m_fixedParams; // We store values of fixed parameters aside
    QVector<QString> m_rows;                             // We store the params name in order of parsing. The order is important (cf some effects like sox)

    std::unique_ptr<Mlt::Properties> m_asset;

    std::shared_ptr<KeyframeModelList> m_keyframes;
    // if true, keyframe tools will be hidden by default
    bool m_hideKeyframesByDefault;

signals:
    void modelChanged();
    /** @brief inform child effects (in case of bin effect with timeline producers)
     *  that a change occurred and a param update is needed **/
    void updateChildren(const QString &name);
    void compositionTrackChanged();
    void replugEffect(std::shared_ptr<AssetParameterModel> asset);
    void rebuildEffect(std::shared_ptr<AssetParameterModel> asset);
    void enabledChange(bool);
};

#endif