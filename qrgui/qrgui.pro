TEMPLATE = subdirs

SUBDIRS += \
	mainWindow \
	systemFacade \
	models \
	editor \
	controller \
	dialogs \
	preferencesDialog \
	textEditor \
	mouseGestures \
	hotKeyManager \
	brandManager \
	pluginManager \
	editorPluginInterface \
	toolPluginInterface \
	constraintsPluginInterface \
	interpretedPluginInterface \
	thirdparty \

pluginManager.file = $$PWD/plugins/pluginManager/pluginManager.pro
editorPluginInterface.file = $$PWD/plugins/editorPluginInterface/editorPluginInterface.pro
toolPluginInterface.file = $$PWD/plugins/toolPluginInterface/toolPluginInterface.pro
interpretedPluginInterface.file = $$PWD/plugins/interpretedPluginInterface/interpretedPluginInterface.pro
constraintsPluginInterface.file = $$PWD/plugins/constraintsPluginInterface/constraintsPluginInterface.pro

mainWindow.depends = \
	systemFacade \
	editor \
	controller \
	dialogs \
	preferencesDialog \
	textEditor \
	mouseGestures \
	hotKeyManager \
	brandManager \
	thirdparty \

systemFacade.depends = \
	models \
	textEditor \
	pluginManager \
	toolPluginInterface \

models.depends = \
	pluginManager \
	controller \

editor.depends = \
	models \
	controller \
	mouseGestures \
	brandManager \
	pluginManager \
	thirdparty \
	dialogs \

dialogs.depends = \
	models \
	thirdparty \

textEditor.depends = \
	toolPluginInterface \

hotKeyManager.depends = \
	preferencesDialog \

brandManager.depends = \
	pluginManager \

pluginManager.depends = \
	constraintsPluginInterface \
	toolPluginInterface \
