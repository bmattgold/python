#include "python.h"
#include "i2capi.h"
#include "structmember.h"

static PyObject* fastspeed;
static PyObject* standardspeed;

static PyObject* exclusivemode;
static PyObject* sharedmode;

#define VALIDATE_I2C(d) \
    if (!(d) || !((d)->ob_device)) { \
        PyErr_SetString(PyExc_ValueError, \
             "I2C device is not initialized"); \
        return NULL; \
    }

static const char moduledocstring[] = "I2C functionality of a Windows 10 IoT Core device";

void define_constants(PyObject *module) {
    fastspeed = Py_BuildValue("i", FASTSPEED);
    PyModule_AddObject(module, "FASTSPEED", fastspeed);

    standardspeed = Py_BuildValue("i", STANDARDSPEED);
    PyModule_AddObject(module, "STANDARDSPEED", standardspeed);

    exclusivemode = Py_BuildValue("i", EXCLUSIVEMODE);
    PyModule_AddObject(module, "EXCLUSIVEMODE", exclusivemode);

    sharedmode = Py_BuildValue("i", SHAREDMODE);
    PyModule_AddObject(module, "SHAREDMODE", sharedmode);
}

static void
i2cdevice_dealloc(PyI2cDeviceObject *d) 
{
    delete_i2cdevice(d->ob_device);
    d->ob_device = NULL;
    Py_TYPE(d)->tp_free((PyObject*)d);
}

static PyObject *
i2cdevice_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyI2cDeviceObject *self;

    self = (PyI2cDeviceObject*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->ob_device = NULL;
    }

    return (PyObject*)self;
}

PyDoc_STRVAR(read_doc,
    "read(count) -> bytes\n"
    "\n"
    "Reads from the device the specified number of bytes\n"
    "\n"
    "count=Number of bytes to read\n"
    );
static PyObject *
wingpio_i2cdevice_read(PyI2cDeviceObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "count", NULL };
    int count = 1;
    PyObject* result = NULL;

    VALIDATE_I2C(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i", kwlist, &count))
        return NULL;

    result = PyBytes_FromStringAndSize(NULL, count);

    if (FAILURE == read_i2cdevice(self->ob_device, PyBytes_AsString(result), count)) {
        Py_DECREF(result);
        result = NULL;
    }

    return result;
}

PyDoc_STRVAR(read_partial_doc,
    "read_partial(count) -> bytes\n"
    "\n"
    "Reads from the device the specified number of bytes\n"
    "\n"
    "count=Number of bytes to read\n"
    );
static PyObject *
wingpio_i2cdevice_read_partial(PyI2cDeviceObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "count", NULL };
    int count = 1;
    PyObject* result = NULL;

    VALIDATE_I2C(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i", kwlist, &count))
        return NULL;

    result = PyBytes_FromStringAndSize(NULL, count);

    count = read_i2cdevice(self->ob_device, PyBytes_AsString(result), count);
    if (count == FAILURE || _PyBytes_Resize(&result, count) == -1) {
        Py_DECREF(result);
        result = NULL;
    }

    return result;
}

PyDoc_STRVAR(write_doc,
    "write(bytes)\n"
    "\n"
    "Writes the bytes to the device\n"
    "\n"
    "bytes=Byte array to be written to the device\n"
    );
static PyObject *
wingpio_i2cdevice_write(PyI2cDeviceObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", NULL };
    PyObject* data = NULL;
    PyObject* bytes = NULL;
    int writeresult = FAILURE;

    VALIDATE_I2C(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|", kwlist, &data))
        return NULL;

    bytes = PyBytes_FromObject(data);
    if (bytes == NULL)
        return NULL;

    writeresult = write_i2cdevice(self->ob_device, PyBytes_AsString(bytes), PyBytes_Size(bytes));
    
    if (writeresult == FAILURE) {
        return NULL;
    }

    Py_RETURN_NONE;
}

PyDoc_STRVAR(write_partial_doc,
    "write_partial(bytes) -> number of bytes written\n"
    "\n"
    "Writes the bytes to the device\n"
    "\n"
    "bytes=Byte array to be written to the device\n"
    );
static PyObject *
wingpio_i2cdevice_write_partial(PyI2cDeviceObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", NULL };
    PyObject* data = NULL;
    PyObject* bytes = NULL;
    int writeresult = FAILURE;

    VALIDATE_I2C(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|", kwlist, &data))
        return NULL;

    bytes = PyBytes_FromObject(data);
    if (bytes == NULL)
        return NULL;

    writeresult = writepartial_i2cdevice(self->ob_device, PyBytes_AsString(bytes), PyBytes_Size(bytes));

    if (writeresult == FAILURE) {
        return NULL;
    }

    return PyLong_FromLong(writeresult);
}

PyDoc_STRVAR(writeread_doc,
    "writeread(bytes, count) -> bytes\n"
    "\n"
    "Writes the bytes to the device and reads the expected number of bytes\n"
    "\n"
    "bytes=Byte array to be written to the device\n"
    "\n"
    "count=Number of bytes expected to be read\n"
    );
static PyObject *
wingpio_i2cdevice_writeread(PyI2cDeviceObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "count", NULL };
    PyObject* data = NULL;
    PyObject* byteArray = NULL;
    int count = 1;
    PyObject* result = NULL;

    VALIDATE_I2C(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i", kwlist, &data, &count))
        return NULL;

    byteArray = PyBytes_FromObject(data);
    if (byteArray == NULL)
        return NULL;

    result = PyBytes_FromStringAndSize(NULL, count);

    if (FAILURE == writeread_i2cdevice(self->ob_device, PyBytes_AsString(byteArray), PyBytes_Size(byteArray), PyBytes_AsString(result), count)) {
        Py_DECREF(result);
        result = NULL;
    }

    return result;
}

PyDoc_STRVAR(writeread_partial_doc,
    "writeread_partial(bytes, count) -> bytes\n"
    "\n"
    "Writes the bytes to the device and reads the expected number of bytes\n"
    "\n"
    "bytes=Byte array to be written to the device\n"
    "\n"
    "count=Number of bytes expected to be read\n"
    );
static PyObject *
wingpio_i2cdevice_writeread_partial(PyI2cDeviceObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "count", NULL };
    PyObject* data = NULL;
    PyObject* byteArray = NULL;
    int count = 1;
    PyObject* result = NULL;

    VALIDATE_I2C(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i", kwlist, &data, &count))
        return NULL;

    byteArray = PyBytes_FromObject(data);
    if (byteArray == NULL)
        return NULL;

    result = PyBytes_FromStringAndSize(NULL, count);

    count = writereadpartial_i2cdevice(self->ob_device, PyBytes_AsString(byteArray), PyBytes_Size(byteArray), PyBytes_AsString(result), count);
    if (count == FAILURE || _PyBytes_Resize(&result, count) == -1) {
        Py_DECREF(result);
        result = NULL;
    }

    return result;
}

PyDoc_STRVAR(deviceid_doc,
    "deviceid() -> string\n"
    "\n"
    "Retrieves the device id for the I2C device\n"
    "\n"
    );
static PyObject *
wingpio_i2cdevice_deviceid(PyI2cDeviceObject *self, PyObject *args) {
    char id[_MAX_FNAME];
    PyObject* result = NULL;
    VALIDATE_I2C(self);

    if (SUCCESS == get_deviceid_i2cdevice(self->ob_device, id, _MAX_FNAME)) {
        result = PyUnicode_FromString(id);
    }

    return result;
}

PyDoc_STRVAR(slaveaddress_doc,
    "slaveaddress() -> int\n"
    "\n"
    "Retrieves the slave address for the I2C device\n"
    "\n"
    );
static PyObject *
wingpio_i2cdevice_slaveaddress(PyI2cDeviceObject *self, PyObject *args)
{
    VALIDATE_I2C(self);
    PyObject* result = NULL;
    int val = 0;

    if (SUCCESS == get_address_i2cdevice(self->ob_device, &val)) {
        result = PyLong_FromLong(val);
    }

    return result;
}

PyDoc_STRVAR(busspeed_doc,
    "busspeed() -> int (FASTSPEED or STANDARDSPEED)\n"
    "\n"
    "Retrieves the bus speed for the I2C device\n"
    "\n"
    );
static PyObject *
wingpio_i2cdevice_busspeed(PyI2cDeviceObject *self, PyObject *args)
{
    VALIDATE_I2C(self);
    PyObject* result = NULL;
    int val = 0;

    if (SUCCESS == get_busspeed_i2cdevice(self->ob_device, &val)) {
        result = PyLong_FromLong(val);
    }

    return result;
}

PyDoc_STRVAR(sharingmode_doc,
    "sharingmode() -> int (EXCLUSIVEMODE or SHAREDMODE)\n"
    "\n"
    "Retrieves the sharing mode for the I2C device\n"
    "\n"
    );
static PyObject *
wingpio_i2cdevice_sharingmode(PyI2cDeviceObject *self, PyObject *args)
{
    VALIDATE_I2C(self);
    PyObject* result = NULL;
    int val = 0;

    if (SUCCESS == get_sharingmode_i2cdevice(self->ob_device, &val)) {
        result = PyLong_FromLong(val);
    }

    return result;
}

static PyMethodDef i2cdevice_methods[] = {
    { "read", (PyCFunction)wingpio_i2cdevice_read, METH_VARARGS | METH_KEYWORDS, read_doc },
    { "read_partial", (PyCFunction)wingpio_i2cdevice_read_partial, METH_VARARGS | METH_KEYWORDS, read_partial_doc },
    { "write", (PyCFunction)wingpio_i2cdevice_write, METH_VARARGS | METH_KEYWORDS, write_doc },
    { "write_partial", (PyCFunction)wingpio_i2cdevice_write_partial, METH_VARARGS | METH_KEYWORDS, write_partial_doc },
    { "writeread", (PyCFunction)wingpio_i2cdevice_writeread, METH_VARARGS | METH_KEYWORDS, writeread_doc },
    { "writeread_partial", (PyCFunction)wingpio_i2cdevice_writeread_partial, METH_VARARGS | METH_KEYWORDS, writeread_partial_doc },
    { "deviceid", (PyCFunction)wingpio_i2cdevice_deviceid, METH_NOARGS, deviceid_doc },
    { "slaveaddress", (PyCFunction)wingpio_i2cdevice_slaveaddress, METH_NOARGS, slaveaddress_doc },
    { "busspeed", (PyCFunction)wingpio_i2cdevice_busspeed, METH_NOARGS, busspeed_doc },
    { "sharingmode", (PyCFunction)wingpio_i2cdevice_sharingmode, METH_NOARGS, sharingmode_doc },
    { NULL, NULL, 0, NULL }
};

static struct PyModuleDef wini2c_module = {
    PyModuleDef_HEAD_INIT,
    "_wini2c",       // name of module
    moduledocstring,  // module documentation, may be NULL
    -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
};

static PyTypeObject i2cdevice_type = {
    PyVarObject_HEAD_INIT(0, 0)
    "_wini2c.i2cdevice",
    sizeof(PyI2cDeviceObject),
    0
};

PyDoc_STRVAR(i2cdevice_doc,
    "i2cdevice(id, slaveaddress, busspeed=STANDARDSPEED, sharingmode=EXCLUSIVEMODE) -> i2cdevice\n"
    "\n"
    "Creates a new instance of an I2C device");
static int
i2cdevice_init(PyI2cDeviceObject *self, PyObject *args, PyObject *kwds)
{
    static char *keywords[] = { "id", "slaveaddress", "busspeed", "sharingmode", NULL };
    int id = 0;
    int slaveAddress = 0;
    int sharingMode = EXCLUSIVEMODE;
    int busSpeed = STANDARDSPEED;
    wchar_t* nameString = NULL;

    if (!PyArg_ParseTupleAndKeywords(args,
            kwds,
            "ii|ii:i2cdevice",
            keywords,
            &id,
            &slaveAddress,
            &busSpeed,
            &sharingMode))
        return -1;

    self->ob_device = new_i2cdevice(id, slaveAddress, busSpeed, sharingMode);

    if (self->ob_device == NULL)
        return FAILURE;

    return SUCCESS;
}

PyMODINIT_FUNC 
PyInit__wini2c(void)
{
    PyObject *module = NULL;

    // Initialize the device type
    i2cdevice_type.tp_dealloc = (destructor)i2cdevice_dealloc;
    i2cdevice_type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    i2cdevice_type.tp_doc = i2cdevice_doc;
    i2cdevice_type.tp_methods = i2cdevice_methods;
    i2cdevice_type.tp_init = (initproc)i2cdevice_init;
    i2cdevice_type.tp_alloc = PyType_GenericAlloc;
    i2cdevice_type.tp_new = i2cdevice_new;
    i2cdevice_type.tp_free = PyObject_Del;
    i2cdevice_type.tp_getattro = PyObject_GenericGetAttr;

    Py_TYPE(&i2cdevice_type) = &PyType_Type;

    if (PyType_Ready(&i2cdevice_type) < 0)
        return NULL;

    if ((module = PyModule_Create(&wini2c_module)) == NULL)
        return NULL;

    Py_INCREF(&i2cdevice_type);
    PyModule_AddObject(module, "i2cdevice", (PyObject*)&i2cdevice_type);

    define_constants(module);

    return module;
}
