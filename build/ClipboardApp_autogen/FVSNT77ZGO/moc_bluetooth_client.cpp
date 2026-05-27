/****************************************************************************
** Meta object code from reading C++ file 'bluetooth_client.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../backend/bluetooth_client.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'bluetooth_client.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_BluetoothClient_t {
    uint offsetsAndSizes[46];
    char stringdata0[16];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[7];
    char stringdata4[8];
    char stringdata5[10];
    char stringdata6[5];
    char stringdata7[13];
    char stringdata8[20];
    char stringdata9[6];
    char stringdata10[18];
    char stringdata11[12];
    char stringdata12[19];
    char stringdata13[21];
    char stringdata14[7];
    char stringdata15[20];
    char stringdata16[11];
    char stringdata17[22];
    char stringdata18[30];
    char stringdata19[14];
    char stringdata20[15];
    char stringdata21[16];
    char stringdata22[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_BluetoothClient_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_BluetoothClient_t qt_meta_stringdata_BluetoothClient = {
    {
        QT_MOC_LITERAL(0, 15),  // "BluetoothClient"
        QT_MOC_LITERAL(16, 15),  // "messageReceived"
        QT_MOC_LITERAL(32, 0),  // ""
        QT_MOC_LITERAL(33, 6),  // "sender"
        QT_MOC_LITERAL(40, 7),  // "message"
        QT_MOC_LITERAL(48, 9),  // "connected"
        QT_MOC_LITERAL(58, 4),  // "name"
        QT_MOC_LITERAL(63, 12),  // "disconnected"
        QT_MOC_LITERAL(76, 19),  // "socketErrorOccurred"
        QT_MOC_LITERAL(96, 5),  // "error"
        QT_MOC_LITERAL(102, 17),  // "devicesDiscovered"
        QT_MOC_LITERAL(120, 11),  // "devicesList"
        QT_MOC_LITERAL(132, 18),  // "onDeviceDiscovered"
        QT_MOC_LITERAL(151, 20),  // "QBluetoothDeviceInfo"
        QT_MOC_LITERAL(172, 6),  // "device"
        QT_MOC_LITERAL(179, 19),  // "onDiscoveryFinished"
        QT_MOC_LITERAL(199, 10),  // "readSocket"
        QT_MOC_LITERAL(210, 21),  // "onSocketErrorOccurred"
        QT_MOC_LITERAL(232, 29),  // "QBluetoothSocket::SocketError"
        QT_MOC_LITERAL(262, 13),  // "connectedSlot"
        QT_MOC_LITERAL(276, 14),  // "startDiscovery"
        QT_MOC_LITERAL(291, 15),  // "connectToDevice"
        QT_MOC_LITERAL(307, 10)   // "addressStr"
    },
    "BluetoothClient",
    "messageReceived",
    "",
    "sender",
    "message",
    "connected",
    "name",
    "disconnected",
    "socketErrorOccurred",
    "error",
    "devicesDiscovered",
    "devicesList",
    "onDeviceDiscovered",
    "QBluetoothDeviceInfo",
    "device",
    "onDiscoveryFinished",
    "readSocket",
    "onSocketErrorOccurred",
    "QBluetoothSocket::SocketError",
    "connectedSlot",
    "startDiscovery",
    "connectToDevice",
    "addressStr"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_BluetoothClient[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   86,    2, 0x06,    1 /* Public */,
       5,    1,   91,    2, 0x06,    4 /* Public */,
       7,    0,   94,    2, 0x06,    6 /* Public */,
       8,    1,   95,    2, 0x06,    7 /* Public */,
      10,    1,   98,    2, 0x06,    9 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      12,    1,  101,    2, 0x08,   11 /* Private */,
      15,    0,  104,    2, 0x08,   13 /* Private */,
      16,    0,  105,    2, 0x08,   14 /* Private */,
      17,    1,  106,    2, 0x08,   15 /* Private */,
      19,    0,  109,    2, 0x08,   17 /* Private */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      20,    0,  110,    2, 0x02,   18 /* Public */,
      21,    1,  111,    2, 0x02,   19 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QVariantList,   11,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 18,    9,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   22,

       0        // eod
};

Q_CONSTINIT const QMetaObject BluetoothClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_BluetoothClient.offsetsAndSizes,
    qt_meta_data_BluetoothClient,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_BluetoothClient_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<BluetoothClient, std::true_type>,
        // method 'messageReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'connected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'disconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'socketErrorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'devicesDiscovered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVariantList &, std::false_type>,
        // method 'onDeviceDiscovered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QBluetoothDeviceInfo &, std::false_type>,
        // method 'onDiscoveryFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'readSocket'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSocketErrorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QBluetoothSocket::SocketError, std::false_type>,
        // method 'connectedSlot'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startDiscovery'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'connectToDevice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void BluetoothClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BluetoothClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->messageReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->connected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->disconnected(); break;
        case 3: _t->socketErrorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->devicesDiscovered((*reinterpret_cast< std::add_pointer_t<QVariantList>>(_a[1]))); break;
        case 5: _t->onDeviceDiscovered((*reinterpret_cast< std::add_pointer_t<QBluetoothDeviceInfo>>(_a[1]))); break;
        case 6: _t->onDiscoveryFinished(); break;
        case 7: _t->readSocket(); break;
        case 8: _t->onSocketErrorOccurred((*reinterpret_cast< std::add_pointer_t<QBluetoothSocket::SocketError>>(_a[1]))); break;
        case 9: _t->connectedSlot(); break;
        case 10: _t->startDiscovery(); break;
        case 11: _t->connectToDevice((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QBluetoothDeviceInfo >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (BluetoothClient::*)(const QString & , const QString & );
            if (_t _q_method = &BluetoothClient::messageReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (BluetoothClient::*)(const QString & );
            if (_t _q_method = &BluetoothClient::connected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (BluetoothClient::*)();
            if (_t _q_method = &BluetoothClient::disconnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (BluetoothClient::*)(const QString & );
            if (_t _q_method = &BluetoothClient::socketErrorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (BluetoothClient::*)(const QVariantList & );
            if (_t _q_method = &BluetoothClient::devicesDiscovered; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
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
    if (!strcmp(_clname, qt_meta_stringdata_BluetoothClient.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BluetoothClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void BluetoothClient::messageReceived(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void BluetoothClient::connected(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void BluetoothClient::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void BluetoothClient::socketErrorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void BluetoothClient::devicesDiscovered(const QVariantList & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
