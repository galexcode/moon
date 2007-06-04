/*
 * moon-plugin-glue.c: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <stdio.h>
#include "npapi.h"
#include "npupp.h"
#include "moon-plugin.h"

static NPNetscapeFuncs MozillaFuncs; // Global function table

/**
 * Wrapper functions
 */
void
NPN_Version (int* plugin_major, int* plugin_minor, int* netscape_major, int* netscape_minor)
{
    *plugin_major = NP_VERSION_MAJOR;
    *plugin_minor = NP_VERSION_MINOR;

    *netscape_major = MozillaFuncs.version >> 8;
    *netscape_minor = MozillaFuncs.version & 0xFF;
}

NPError
NPN_GetValue (NPP instance, NPNVariable variable, void *r_value)
{
	return CallNPN_GetValueProc (MozillaFuncs.getvalue, instance, variable, r_value);
}

NPError
NPN_SetValue (NPP instance, NPPVariable variable, void *value)
{
	return CallNPN_SetValueProc (MozillaFuncs.setvalue, instance, variable, value);
}

NPError
NPN_GetURL (NPP instance, const char* url, const char* window)
{
	return CallNPN_GetURLProc (MozillaFuncs.geturl, instance, url, window);
}

NPError
NPN_GetURLNotify (NPP instance, const char* url, const char* window, void* notifyData)
{
	return CallNPN_GetURLNotifyProc (MozillaFuncs.geturlnotify, instance, url, window, notifyData);
}

NPError
NPN_PostURL (NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file)
{
	return CallNPN_PostURLProc (MozillaFuncs.posturl, instance, url, window, len, buf, file);
}

NPError
NPN_PostURLNotify (NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file, void* notifyData)
{
	return CallNPN_PostURLNotifyProc (MozillaFuncs.posturlnotify, instance, url, window, len, buf, file, notifyData);
}

NPError
NPN_RequestRead (NPStream* stream, NPByteRange* rangeList)
{
	return CallNPN_RequestReadProc (MozillaFuncs.requestread, stream, rangeList);
}

NPError
NPN_NewStream (NPP instance, NPMIMEType type, const char *window, NPStream** stream_ptr)
{
	return CallNPN_NewStreamProc (MozillaFuncs.newstream, instance, type, window, stream_ptr);
}

int32
NPN_Write (NPP instance, NPStream* stream, int32 len, void* buffer)
{
	return CallNPN_WriteProc (MozillaFuncs.write, instance, stream, len, buffer);
}

NPError
NPN_DestroyStream (NPP instance, NPStream* stream, NPError reason)
{
	return CallNPN_DestroyStreamProc (MozillaFuncs.destroystream, instance, stream, reason);
}

void
NPN_Status (NPP instance, const char* message)
{
	CallNPN_StatusProc (MozillaFuncs.status, instance, message);
}

const char*
NPN_UserAgent (NPP instance)
{
	return CallNPN_UserAgentProc (MozillaFuncs.uagent, instance);
}

void*
NPN_MemAlloc (uint32 size)
{
	return CallNPN_MemAllocProc (MozillaFuncs.memalloc, size);
}

void NPN_MemFree (void* ptr)
{
	CallNPN_MemFreeProc (MozillaFuncs.memfree, ptr);
}

uint32 NPN_MemFlush (uint32 size)
{
	return CallNPN_MemFlushProc (MozillaFuncs.memflush, size);
}

void NPN_ReloadPlugins (NPBool reloadPages)
{
	CallNPN_ReloadPluginsProc (MozillaFuncs.reloadplugins, reloadPages);
}

void
NPN_InvalidateRect (NPP instance, NPRect *invalidRect)
{
	CallNPN_InvalidateRectProc (MozillaFuncs.invalidaterect, instance, invalidRect);
}

void
NPN_InvalidateRegion (NPP instance, NPRegion invalidRegion)
{
	CallNPN_InvalidateRegionProc (MozillaFuncs.invalidateregion, instance, invalidRegion);
}

void
NPN_ForceRedraw (NPP instance)
{
	CallNPN_ForceRedrawProc (MozillaFuncs.forceredraw, instance);
}

void NPN_PushPopupsEnabledState (NPP instance, NPBool enabled)
{
	CallNPN_PushPopupsEnabledStateProc (MozillaFuncs.pushpopupsenabledstate, instance, enabled);
}

void NPN_PopPopupsEnabledState (NPP instance)
{
	CallNPN_PopPopupsEnabledStateProc (MozillaFuncs.poppopupsenabledstate, instance);
}

/**
 * Plugin functions
 */
NPError 
plugin_new (NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
	DEBUG ("plugin_new");

	PluginInstance *plugin = new PluginInstance ();

	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

    instance->pdata = NPN_MemAlloc (sizeof (PluginInstance));

    plugin = (PluginInstance*) instance->pdata;

    if (plugin == NULL) 
    {
        return NPERR_OUT_OF_MEMORY_ERROR;
    }

    memset (plugin, 0, sizeof (PluginInstance));

    plugin->mode = mode;
    plugin->instance = instance;

	return NPERR_NO_ERROR;
}

NPError 
plugin_destroy (NPP instance, NPSavedData** save)
{
	DEBUG ("plugin_destroy");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	delete plugin;

	return NPERR_NO_ERROR;
}

NPError 
plugin_get_value (NPP instance, NPPVariable variable, void *result)
{
	DEBUG ("plugin_get_value %d (%x)", variable, variable);
	
	NPError err;

	switch (variable) {
		case NPPVpluginNameString:
			*((char **)result) = PLUGIN_NAME;
			break;
		case NPPVpluginDescriptionString:
			*((char **)result) = PLUGIN_DESCRIPTION;
			break;
		case NPPVpluginNeedsXEmbed:
			*((PRBool *)result) = PR_TRUE;
			break;
		default:
			if (instance != NULL) {
				PluginInstance *plugin = (PluginInstance *) instance->pdata;
				err = plugin->GetValue (variable, result);
			} else {
				err = NPERR_GENERIC_ERROR;
			}
	}
	return err;
}

NPError 
plugin_set_window (NPP instance, NPWindow* window)
{
	DEBUG ("plugin_set_window %d %d", window->width, window->height);

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->SetWindow (window);
}

NPError
plugin_new_stream (NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	DEBUG ("plugin_new_stream");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->NewStream (type, stream, seekable, stype);
}

void
plugin_stream_as_file (NPP instance, NPStream* stream, const char* fname)
{
	DEBUG ("plugin_stream_as_file");

	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->StreamAsFile (stream, fname);
}

NPError
plugin_destroy_stream (NPP instance, NPStream* stream, NPError reason)
{
	DEBUG ("plugin_destroy_stream");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->DestroyStream (stream, reason);
}

int32
plugin_write_ready (NPP instance, NPStream* stream)
{
	DEBUG ("plugin_write_ready");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	NPN_DestroyStream (instance, stream, NPRES_DONE);

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->WriteReady (stream);
}

int32
plugin_write (NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer)
{
	DEBUG ("plugin_write");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	NPN_DestroyStream (instance, stream, NPRES_DONE);

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->Write (stream, offset, len, buffer);
}

void
plugin_url_notify (NPP instance, const char* url, NPReason reason, void* notifyData)
{
	DEBUG ("plugin_url_notify");

	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->UrlNotify (url, reason, notifyData);
}

void
plugin_print (NPP instance, NPPrint* platformPrint)
{
	DEBUG ("plugin_print");

	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->Print (platformPrint);
}

int16 
plugin_handle_event (NPP instance, void* event)
{
	DEBUG ("*** plugin_handle_event (not implemented)");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->EventHandle (event);
}

/**
 * These functions are located automagically by mozilla
 */
NPError
NP_GetValue (void* future, NPPVariable variable, void *value)
{
	DEBUG ("NP_GetValue %d (%x)", variable, variable);

	return plugin_get_value (NULL, variable, value);
}

char *
NP_GetMIMEDescription (void)
{
	DEBUG ("NP_GetMIMEDescription");
    return (MIME_TYPES_HANDLED);
}

NPError
NP_Initialize (NPNetscapeFuncs * mozilla_funcs, NPPluginFuncs * plugin_funcs)
{
	NPError err = NPERR_NO_ERROR;
	NPBool supportsXEmbed = PR_FALSE;
	NPNToolkitType toolkit = (NPNToolkitType) 0;

	DEBUG ("NP_Initialize");

	/* XEMBED? */
	err = CallNPN_GetValueProc (mozilla_funcs->getvalue, NULL,
								NPNVSupportsXEmbedBool,
								(void *)&supportsXEmbed);

	if (err != NPERR_NO_ERROR || supportsXEmbed != PR_TRUE)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;

	/* GTK+ 2.x Moz? */
	err = CallNPN_GetValueProc (mozilla_funcs->getvalue, NULL,
								NPNVToolkit, 
								(void *)&toolkit);

	if (err != NPERR_NO_ERROR || toolkit != NPNVGtk2)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;

	if (mozilla_funcs == NULL || plugin_funcs == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	if ((mozilla_funcs->version >> 8) > NP_VERSION_MAJOR)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;

	if (mozilla_funcs->size < sizeof (NPNetscapeFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;

	if (plugin_funcs->size < sizeof (NPPluginFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;

	/* Copy fields from Mozilla function table to global MozillaFuncs */
	MozillaFuncs.version                = mozilla_funcs->version;
	MozillaFuncs.size                   = mozilla_funcs->size;
	MozillaFuncs.posturl                = mozilla_funcs->posturl;
	MozillaFuncs.geturl                 = mozilla_funcs->geturl;
	MozillaFuncs.geturlnotify           = mozilla_funcs->geturlnotify;
	MozillaFuncs.requestread            = mozilla_funcs->requestread;
	MozillaFuncs.newstream              = mozilla_funcs->newstream;
	MozillaFuncs.write                  = mozilla_funcs->write;
	MozillaFuncs.destroystream          = mozilla_funcs->destroystream;
	MozillaFuncs.status                 = mozilla_funcs->status;
	MozillaFuncs.uagent                 = mozilla_funcs->uagent;
	MozillaFuncs.memalloc               = mozilla_funcs->memalloc;
	MozillaFuncs.memfree                = mozilla_funcs->memfree;
	MozillaFuncs.memflush               = mozilla_funcs->memflush;
	MozillaFuncs.reloadplugins          = mozilla_funcs->reloadplugins;
	MozillaFuncs.getJavaEnv             = mozilla_funcs->getJavaEnv;
	MozillaFuncs.getJavaPeer            = mozilla_funcs->getJavaPeer;
	MozillaFuncs.getvalue               = mozilla_funcs->getvalue;
	MozillaFuncs.setvalue               = mozilla_funcs->setvalue;
	MozillaFuncs.pushpopupsenabledstate = mozilla_funcs->pushpopupsenabledstate;
	MozillaFuncs.poppopupsenabledstate  = mozilla_funcs->poppopupsenabledstate;

	/* Set up the plugin function table */
	plugin_funcs->size          = sizeof (NPPluginFuncs);
	plugin_funcs->version       = (NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR;
	plugin_funcs->newp          = NewNPP_NewProc (plugin_new);
	plugin_funcs->destroy       = NewNPP_DestroyProc (plugin_destroy);
	plugin_funcs->getvalue      = NewNPP_GetValueProc (plugin_get_value);
	plugin_funcs->setwindow     = NewNPP_SetWindowProc (plugin_set_window);
	plugin_funcs->newstream     = NewNPP_NewStreamProc (plugin_new_stream);
	plugin_funcs->asfile        = NewNPP_StreamAsFileProc (plugin_stream_as_file);
	plugin_funcs->destroystream = NewNPP_DestroyStreamProc (plugin_destroy_stream);
	plugin_funcs->writeready    = NewNPP_WriteReadyProc (plugin_write_ready);
	plugin_funcs->write         = NewNPP_WriteProc (plugin_write);
	plugin_funcs->print         = NewNPP_PrintProc (plugin_print);
	plugin_funcs->urlnotify     = NewNPP_URLNotifyProc (plugin_url_notify);
	plugin_funcs->event         = NewNPP_HandleEventProc (plugin_handle_event);
	plugin_funcs->javaClass     = NULL;

	gtk_init (0, 0);

	DEBUG ("NP_Initialize succeeded");

	return NPERR_NO_ERROR;
}

NPError
NP_Shutdown (void)
{
	DEBUG ("NP_Shutdown");
	return NPERR_NO_ERROR;
}
