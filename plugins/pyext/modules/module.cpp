#include <KNotification>

#include "module.h"
namespace {
    PyObject* createSimpleNotification(PyObject* self, PyObject* text) {
        PyObject* strObj = PyObject_Str(text);
        const char* s = PyUnicode_AS_DATA(strObj);
        
        KNotification* notification = new KNotification("notification", KNotification::CloseOnTimeout);
        notification->setIconName(QStringLiteral("preferences-desktop-notification"));
        notification->setComponentName("kdeconnect");
        notification->setTitle("Title");
        notification->setText(QString(s));
        notification->sendEvent();
        
        Py_DecRef(strObj);
        
        return NULL;
    }
}

PyMethodDef module_exports[] = {
    {"create_simple_notification", createSimpleNotification, METH_VARARGS, "docstring"},
    {NULL, NULL, 0, NULL}
};
