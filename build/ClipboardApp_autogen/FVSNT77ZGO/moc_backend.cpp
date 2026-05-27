/****************************************************************************
** Meta object code from reading C++ file 'backend.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../backend/backend.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'backend.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_Backend_t {
    uint offsetsAndSizes[20];
    char stringdata0[8];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[19];
    char stringdata5[17];
    char stringdata6[5];
    char stringdata7[11];
    char stringdata8[13];
    char stringdata9[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Backend_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Backend_t qt_meta_stringdata_Backend = {
    {
        QT_MOC_LITERAL(0, 7),  // "Backend"
        QT_MOC_LITERAL(8, 9),  // "itemAdded"
        QT_MOC_LITERAL(18, 0),  // ""
        QT_MOC_LITERAL(19, 4),  // "item"
        QT_MOC_LITERAL(24, 18),  // "onClipboardChanged"
        QT_MOC_LITERAL(43, 16),  // "handleRemoteData"
        QT_MOC_LITERAL(60, 4),  // "data"
        QT_MOC_LITERAL(65, 10),  // "copyToClip"
        QT_MOC_LITERAL(76, 12),  // "clearHistory"
        QT_MOC_LITERAL(89, 17)   // "getGlobalMousePos"
    },
    "Backend",
    "itemAdded",
    "",
    "item",
    "onClipboardChanged",
    "handleRemoteData",
    "data",
    "copyToClip",
    "clearHistory",
    "getGlobalMousePos"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Backend[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   50,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       4,    0,   53,    2, 0x0a,    3 /* Public */,
       5,    1,   54,    2, 0x0a,    4 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       7,    1,   57,    2, 0x02,    6 /* Public */,
       8,    0,   60,    2, 0x02,    8 /* Public */,
       9,    0,   61,    2, 0x02,    9 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QVariant,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QVariantMap,    6,

 // methods: parameters
    QMetaType::Void, QMetaType::QVariant,    6,
    QMetaType::Void,
    QMetaType::QPoint,

       0        // eod
};

Q_CONSTINIT const QMetaObject Backend::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Backend.offsetsAndSizes,
    qt_meta_data_Backend,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Backend_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Backend, std::true_type>,
        // method 'itemAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QVariant, std::false_type>,
        // method 'onClipboardChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleRemoteData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QVariantMap, std::false_type>,
        // method 'copyToClip'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QVariant, std::false_type>,
        // method 'clearHistory'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'getGlobalMousePos'
        QtPrivate::TypeAndForceComplete<QPoint, std::false_type>
    >,
    nullptr
} };

void Backend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Backend *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->itemAdded((*reinterpret_cast< std::add_pointer_t<QVariant>>(_a[1]))); break;
        case 1: _t->onClipboardChanged(); break;
        case 2: _t->handleRemoteData((*reinterpret_cast< std::add_pointer_t<QVariantMap>>(_a[1]))); break;
        case 3: _t->copyToClip((*reinterpret_cast< std::add_pointer_t<QVariant>>(_a[1]))); break;
        case 4: _t->clearHistory(); break;
        case 5: { QPoint _r = _t->getGlobalMousePos();
            if (_a[0]) *reinterpret_cast< QPoint*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Backend::*)(QVariant );
            if (_t _q_method = &Backend::itemAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *Backend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Backend::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Backend.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Backend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void Backend::itemAdded(QVariant _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
