/****************************************************************************
** Meta object code from reading C++ file 'bluetooth_client.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../Clipboard/backend/ble/bluetooth_client.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'bluetooth_client.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN15BluetoothClientE_t {};
} // unnamed namespace

template <> constexpr inline auto BluetoothClient::qt_create_metaobjectdata<qt_meta_tag_ZN15BluetoothClientE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BluetoothClient",
        "connectionRequested",
        "",
        "disconnectRequested",
        "messageReceived",
        "sender",
        "message",
        "dataReceived",
        "QVariantMap",
        "item",
        "transferError",
        "error",
        "transferFinished",
        "connected",
        "name",
        "disconnected",
        "socketErrorOccurred",
        "devicesDiscovered",
        "QVariantList",
        "devicesList",
        "discoveringChanged",
        "stateChanged",
        "onDeviceDiscovered",
        "QBluetoothDeviceInfo",
        "device",
        "onDiscoveryFinished",
        "onSocketErrorOccurred",
        "QBluetoothSocket::SocketError",
        "connectedSlot",
        "onSocketDisconnected",
        "startDiscovery",
        "connectToDevice",
        "addressStr",
        "disconnectFromDevice",
        "devices",
        "discovering",
        "connecting",
        "connectedAddress"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'connectionRequested'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'disconnectRequested'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'messageReceived'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { QMetaType::QString, 6 },
        }}),
        // Signal 'dataReceived'
        QtMocHelpers::SignalData<void(const QVariantMap &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Signal 'transferError'
        QtMocHelpers::SignalData<void(const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Signal 'transferFinished'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'connected'
        QtMocHelpers::SignalData<void(const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Signal 'disconnected'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'socketErrorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Signal 'devicesDiscovered'
        QtMocHelpers::SignalData<void(const QVariantList &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 18, 19 },
        }}),
        // Signal 'discoveringChanged'
        QtMocHelpers::SignalData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'stateChanged'
        QtMocHelpers::SignalData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onDeviceDiscovered'
        QtMocHelpers::SlotData<void(const QBluetoothDeviceInfo &)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 23, 24 },
        }}),
        // Slot 'onDiscoveryFinished'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSocketErrorOccurred'
        QtMocHelpers::SlotData<void(QBluetoothSocket::SocketError)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 27, 11 },
        }}),
        // Slot 'connectedSlot'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSocketDisconnected'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'startDiscovery'
        QtMocHelpers::MethodData<void()>(30, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'connectToDevice'
        QtMocHelpers::MethodData<void(const QString &)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 32 },
        }}),
        // Method 'disconnectFromDevice'
        QtMocHelpers::MethodData<void()>(33, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'devices'
        QtMocHelpers::PropertyData<QVariantList>(34, 0x80000000 | 18, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 9),
        // property 'discovering'
        QtMocHelpers::PropertyData<bool>(35, QMetaType::Bool, QMC::DefaultPropertyFlags, 10),
        // property 'connecting'
        QtMocHelpers::PropertyData<bool>(36, QMetaType::Bool, QMC::DefaultPropertyFlags, 11),
        // property 'connectedAddress'
        QtMocHelpers::PropertyData<QString>(37, QMetaType::QString, QMC::DefaultPropertyFlags, 11),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<BluetoothClient, qt_meta_tag_ZN15BluetoothClientE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BluetoothClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15BluetoothClientE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15BluetoothClientE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15BluetoothClientE_t>.metaTypes,
    nullptr
} };

void BluetoothClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BluetoothClient *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectionRequested(); break;
        case 1: _t->disconnectRequested(); break;
        case 2: _t->messageReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->dataReceived((*reinterpret_cast< std::add_pointer_t<QVariantMap>>(_a[1]))); break;
        case 4: _t->transferError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->transferFinished(); break;
        case 6: _t->connected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->disconnected(); break;
        case 8: _t->socketErrorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->devicesDiscovered((*reinterpret_cast< std::add_pointer_t<QVariantList>>(_a[1]))); break;
        case 10: _t->discoveringChanged(); break;
        case 11: _t->stateChanged(); break;
        case 12: _t->onDeviceDiscovered((*reinterpret_cast< std::add_pointer_t<QBluetoothDeviceInfo>>(_a[1]))); break;
        case 13: _t->onDiscoveryFinished(); break;
        case 14: _t->onSocketErrorOccurred((*reinterpret_cast< std::add_pointer_t<QBluetoothSocket::SocketError>>(_a[1]))); break;
        case 15: _t->connectedSlot(); break;
        case 16: _t->onSocketDisconnected(); break;
        case 17: _t->startDiscovery(); break;
        case 18: _t->connectToDevice((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 19: _t->disconnectFromDevice(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QBluetoothDeviceInfo >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)()>(_a, &BluetoothClient::connectionRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)()>(_a, &BluetoothClient::disconnectRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)(const QString & , const QString & )>(_a, &BluetoothClient::messageReceived, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)(const QVariantMap & )>(_a, &BluetoothClient::dataReceived, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)(const QString & )>(_a, &BluetoothClient::transferError, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)()>(_a, &BluetoothClient::transferFinished, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)(const QString & )>(_a, &BluetoothClient::connected, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)()>(_a, &BluetoothClient::disconnected, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)(const QString & )>(_a, &BluetoothClient::socketErrorOccurred, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)(const QVariantList & )>(_a, &BluetoothClient::devicesDiscovered, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)()>(_a, &BluetoothClient::discoveringChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (BluetoothClient::*)()>(_a, &BluetoothClient::stateChanged, 11))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QVariantList*>(_v) = _t->devices(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->isDiscovering(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->isConnecting(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->connectedAddress(); break;
        default: break;
        }
    }
}

const QMetaObject *BluetoothClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BluetoothClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15BluetoothClientE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BluetoothClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void BluetoothClient::connectionRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void BluetoothClient::disconnectRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void BluetoothClient::messageReceived(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void BluetoothClient::dataReceived(const QVariantMap & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void BluetoothClient::transferError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void BluetoothClient::transferFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void BluetoothClient::connected(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void BluetoothClient::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void BluetoothClient::socketErrorOccurred(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void BluetoothClient::devicesDiscovered(const QVariantList & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void BluetoothClient::discoveringChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void BluetoothClient::stateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}
QT_WARNING_POP
