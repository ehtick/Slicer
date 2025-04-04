import os
import traceback

import ctk
import qt

import slicer

import SlicerWizard.ExtensionDescription
import SlicerWizard.ExtensionProject
import SlicerWizard.TemplateManager
import SlicerWizard.Utilities

from ExtensionWizardLib import *

from slicer.i18n import tr as _
from slicer.i18n import translate


# -----------------------------------------------------------------------------
def _settingsList(settings, key, convertToAbsolutePaths=False):
    # Return a settings value as a list (even if empty or a single value)

    value = settings.value(key)
    if value is None:
        return []
    if isinstance(value, str):
        value = [value]

    if convertToAbsolutePaths:
        absolutePaths = []
        for path in value:
            absolutePaths.append(slicer.app.toSlicerHomeAbsolutePath(path))
        return absolutePaths
    else:
        return value


# =============================================================================
#
# ExtensionWizard
#
# =============================================================================
class ExtensionWizard:
    # ---------------------------------------------------------------------------
    def __init__(self, parent):
        parent.title = _("Extension Wizard")
        parent.icon = qt.QIcon(":/Icons/Medium/ExtensionWizard.png")
        parent.categories = [translate("qSlicerAbstractCoreModule", "Developer Tools")]
        parent.dependencies = []
        parent.contributors = ["Matthew Woehlke (Kitware)"]
        parent.helpText = _("This module provides tools to create and manage extensions from within Slicer.")
        parent.acknowledgementText = _("This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community.")
        self.parent = parent

        self.settingsPanel = SettingsPanel()
        slicer.app.settingsDialog().addPanel("Extension Wizard", self.settingsPanel)


# =============================================================================
#
# ExtensionWizardWidget
#
# =============================================================================
class ExtensionWizardWidget:
    # ---------------------------------------------------------------------------
    def __init__(self, parent=None):
        if not parent:
            self.parent = qt.QWidget()
            self.parent.setLayout(qt.QVBoxLayout())

        else:
            self.parent = parent

        self.layout = self.parent.layout()

        if not parent:
            self.setup()
            self.parent.show()

        self.extensionProject = None
        self.extensionDescription = None
        self.extensionLocation = None

        self.templateManager = None
        self.setupTemplates()

    # ---------------------------------------------------------------------------
    def setup(self):
        # Instantiate and connect widgets ...

        icon = self.parent.style().standardIcon(qt.QStyle.SP_ArrowForward)
        iconSize = qt.QSize(22, 22)

        def createToolButton(text):
            tb = qt.QToolButton()

            tb.text = text
            tb.icon = icon

            font = tb.font
            font.setBold(True)
            font.setPixelSize(14)
            tb.font = font

            tb.iconSize = iconSize
            tb.toolButtonStyle = qt.Qt.ToolButtonTextBesideIcon
            tb.autoRaise = True

            return tb

        def createReadOnlyLineEdit():
            le = qt.QLineEdit()
            le.readOnly = True
            le.frame = False
            le.styleSheet = "QLineEdit { background:transparent; }"
            le.cursor = qt.QCursor(qt.Qt.IBeamCursor)
            return le

        #
        # Tools Area
        #
        self.toolsCollapsibleButton = ctk.ctkCollapsibleButton()
        self.toolsCollapsibleButton.text = _("Extension Tools")
        self.layout.addWidget(self.toolsCollapsibleButton)

        self.createExtensionButton = createToolButton(_("Create Extension"))
        self.createExtensionButton.connect("clicked(bool)", self.createExtension)

        self.selectExtensionButton = createToolButton(_("Select Extension"))
        self.selectExtensionButton.connect("clicked(bool)", self.selectExtension)

        toolsLayout = qt.QVBoxLayout(self.toolsCollapsibleButton)
        toolsLayout.addWidget(self.createExtensionButton)
        toolsLayout.addWidget(self.selectExtensionButton)

        #
        # Editor Area
        #
        self.editorCollapsibleButton = ctk.ctkCollapsibleButton()
        self.editorCollapsibleButton.text = _("Extension Editor")
        self.editorCollapsibleButton.enabled = False
        self.editorCollapsibleButton.collapsed = True
        self.layout.addWidget(self.editorCollapsibleButton)

        self.extensionNameField = createReadOnlyLineEdit()
        self.extensionLocationField = createReadOnlyLineEdit()
        self.extensionRepositoryField = createReadOnlyLineEdit()

        self.extensionContentsModel = qt.QFileSystemModel()
        self.extensionContentsView = qt.QTreeView()
        self.extensionContentsView.setModel(self.extensionContentsModel)
        self.extensionContentsView.sortingEnabled = True
        self.extensionContentsView.hideColumn(3)

        self.createExtensionModuleButton = createToolButton(_("Add Module to Extension"))
        self.createExtensionModuleButton.connect("clicked(bool)",
                                                 self.createExtensionModule)

        self.editExtensionMetadataButton = createToolButton(_("Edit Extension Metadata"))
        self.editExtensionMetadataButton.connect("clicked(bool)",
                                                 self.editExtensionMetadata)

        editorLayout = qt.QFormLayout(self.editorCollapsibleButton)
        editorLayout.addRow(_("Name:"), self.extensionNameField)
        editorLayout.addRow(_("Location:"), self.extensionLocationField)
        editorLayout.addRow(_("Repository:"), self.extensionRepositoryField)
        editorLayout.addRow(_("Contents:"), self.extensionContentsView)
        editorLayout.addRow(self.createExtensionModuleButton)
        editorLayout.addRow(self.editExtensionMetadataButton)

        # Add vertical spacer
        self.layout.addStretch(1)

    # ---------------------------------------------------------------------------
    def cleanup(self):
        pass

    # ---------------------------------------------------------------------------
    def setupTemplates(self):
        self.templateManager = SlicerWizard.TemplateManager()

        builtinPath = builtinTemplatePath()
        if builtinPath is not None:
            try:
                self.templateManager.addPath(builtinPath)
            except:
                qt.qWarning(f"failed to add built-in template {builtinPath}")
                qt.qWarning(traceback.format_exc())

        # Read base template paths
        s = qt.QSettings()
        for path in _settingsList(s, userTemplatePathKey(), convertToAbsolutePaths=True):
            try:
                self.templateManager.addPath(path)
            except:
                qt.qWarning(f"failed to add template {path}")
                qt.qWarning(traceback.format_exc())

        # Read per-category template paths
        s.beginGroup(userTemplatePathKey())
        for c in s.allKeys():
            for path in _settingsList(s, c, convertToAbsolutePaths=True):
                try:
                    self.templateManager.addCategoryPath(c, path)
                except:
                    mp = (c, path)
                    qt.qWarning(f"failed to add template {path} for category {c}")
                    qt.qWarning(traceback.format_exc())

    # ---------------------------------------------------------------------------
    def createExtension(self):
        dlg = CreateComponentDialog("extension", self.parent.window())
        dlg.setTemplates(self.templateManager.templates("extensions"))

        while dlg.exec_() == qt.QDialog.Accepted:
            # If the selected destination is in a repository then use the root of that repository
            # as destination
            try:
                repo = SlicerWizard.Utilities.getRepo(dlg.destination)

                createInSubdirectory = True
                requireEmptyDirectory = True

                if repo is None:
                    destination = os.path.join(dlg.destination, dlg.componentName)
                    if os.path.exists(destination):
                        raise OSError("create extension: refusing to overwrite"
                                      " existing directory '%s'" % destination)
                    createInSubdirectory = False

                else:
                    destination = SlicerWizard.Utilities.localRoot(repo)
                    cmakeFile = os.path.join(destination, "CMakeLists.txt")
                    createInSubdirectory = False  # create the files in the destination directory
                    requireEmptyDirectory = False  # we only check if no CMakeLists.txt file exists
                    if os.path.exists(cmakeFile):
                        raise OSError("create extension: refusing to overwrite"
                                      " directory containing CMakeLists.txt file at '%s'" % dlg.destination)

                path = self.templateManager.copyTemplate(
                    destination, "extensions",
                    dlg.componentType, dlg.componentName,
                    createInSubdirectory, requireEmptyDirectory)

            except:
                if not slicer.util.confirmRetryCloseDisplay(_("An error occurred while trying to create the extension."),
                                                            parent=self.parent.window(), detailedText=traceback.format_exc()):
                    return

                continue

            if self.selectExtension(path):
                self.editExtensionMetadata()

            return

    # ---------------------------------------------------------------------------
    def selectExtension(self, path=None):
        if path is None or isinstance(path, bool):
            path = qt.QFileDialog.getExistingDirectory(
                self.parent.window(), _("Select Extension..."),
                self.extensionLocation)

        if not len(path):
            return False

        # Attempt to open extension
        try:
            repo = SlicerWizard.Utilities.getRepo(path)

            xd = None
            if repo:
                try:
                    xd = SlicerWizard.ExtensionDescription(repo=repo)
                    path = SlicerWizard.Utilities.localRoot(repo)
                except:
                    # Failed to determine repository path automatically (git is not installed, etc.)
                    # Continue with assuming that the user selected the top-level directory of the extension.
                    pass

            if not xd:
                xd = SlicerWizard.ExtensionDescription(sourcedir=path)

            xp = SlicerWizard.ExtensionProject(path)

        except:
            slicer.util.errorDisplay(_("Failed to open extension {path}.").format(path=path), parent=self.parent.window(),
                                     detailedText=traceback.format_exc(), standardButtons=qt.QMessageBox.Close)
            return False

        # Enable and show edit section
        self.editorCollapsibleButton.enabled = True
        self.editorCollapsibleButton.collapsed = False

        # Populate edit information
        self.extensionNameField.text = xp.project
        self.extensionLocationField.text = path

        if xd.scmurl == "NA":
            if repo is None:
                repoText = _("(none)")
            elif hasattr(repo, "remotes"):
                repoText = _("(local git repository)")
            else:
                repoText = _("(unknown local repository)")

            self.extensionRepositoryField.clear()
            self.extensionRepositoryField.placeholderText = repoText

        else:
            self.extensionRepositoryField.text = xd.scmurl

        ri = self.extensionContentsModel.setRootPath(path)
        self.extensionContentsView.setRootIndex(ri)

        w = self.extensionContentsView.width
        self.extensionContentsView.setColumnWidth(0, int((w * 4) / 9))

        # Prompt to load scripted modules from extension
        ExtensionWizardWidget.loadModules(path, parent=self.parent.window())

        # Store extension location, project and description for later use
        self.extensionProject = xp
        self.extensionDescription = xd
        self.extensionLocation = path
        return True

    # ---------------------------------------------------------------------------
    @staticmethod
    def loadModules(path=None, depth=1, modules=None, parent=None):
        if parent is None:
            parent = slicer.util.mainWindow()
        if path is not None:
            # Get list of modules in specified path
            modules = ModuleInfo.findModules(path, depth)
        elif modules is None:
            raise RuntimeError("loadModules require 'path' or 'modules' input")

        # Determine which modules in above are not already loaded
        factory = slicer.app.moduleManager().factoryManager()
        loadedModules = factory.instantiatedModuleNames()

        candidates = [m for m in modules if m.key not in loadedModules]

        # Prompt to load additional module(s)
        if candidates:
            dlg = LoadModulesDialog(parent)
            dlg.setModules(candidates)

            if dlg.exec_() == qt.QDialog.Accepted:
                modulesToLoad = dlg.selectedModules

                # Add module(s) to permanent search paths, if requested
                if dlg.addToSearchPaths:
                    settings = slicer.app.revisionUserSettings()
                    rawSearchPaths = list(_settingsList(settings, "Modules/AdditionalPaths", convertToAbsolutePaths=True))
                    searchPaths = [qt.QDir(path) for path in rawSearchPaths]
                    modified = False

                    for module in modulesToLoad:
                        rawPath = os.path.dirname(module.path)
                        path = qt.QDir(rawPath)
                        if path not in searchPaths:
                            searchPaths.append(path)
                            rawSearchPaths.append(rawPath)
                            modified = True

                    if modified:
                        settings.setValue("Modules/AdditionalPaths", slicer.app.toSlicerHomeRelativePaths(rawSearchPaths))

                # Enable developer mode (shows Reload&Test section, etc.), if requested
                if dlg.enableDeveloperMode:
                    qt.QSettings().setValue("Developer/DeveloperMode", "true")

                # Register requested module(s)
                failed = []

                for module in modulesToLoad:
                    factory.registerModule(qt.QFileInfo(module.path))
                    if not factory.isRegistered(module.key):
                        failed.append(module)

                if failed:
                    if len(failed) > 1:
                        text = _("{count} modules could not be registered").format(count=count)
                    else:
                        text = _("The {name} module could not be registered").format(name=failed[0].key)

                    failedFormat = "<ul><li>%(key)s<br/>(%(path)s)</li></ul>"
                    detailedInformation = "".join(
                        [failedFormat % m.__dict__ for m in failed])

                    slicer.util.errorDisplay(text, parent=parent, windowTitle=_("Module loading failed"),
                                             standardButtons=qt.QMessageBox.Close, informativeText=detailedInformation)

                    return

                # Instantiate and load requested module(s)
                if not factory.loadModules([module.key for module in modulesToLoad]):
                    text = _("The module factory manager reported an error. "
                            "One or more of the requested module(s) and/or "
                            "dependencies thereof may not have been loaded.")
                    slicer.util.errorDisplay(text, parent=parent, windowTitle=_("Error loading module(s)"),
                                             standardButtons=qt.QMessageBox.Close)

    # ---------------------------------------------------------------------------
    def createExtensionModule(self):
        if self.extensionLocation is None:
            # Action shouldn't be enabled if no extension is selected, but guard
            # against that just in case...
            return

        dlg = CreateComponentDialog("module", self.parent.window())
        dlg.setTemplates(self.templateManager.templates("modules"),
                         default="scripted")
        dlg.showDestination = False

        while dlg.exec_() == qt.QDialog.Accepted:
            name = dlg.componentName

            try:
                self.templateManager.copyTemplate(self.extensionLocation, "modules",
                                                  dlg.componentType, name)

            except:
                if not slicer.util.confirmRetryCloseDisplay(_("An error occurred while trying to create the module."),
                                                            parent=self.parent.window(),
                                                            detailedText=traceback.format_exc()):
                    return

                continue

            try:
                self.extensionProject.addModule(name)
                self.extensionProject.save()

            except:
                text = _("An error occurred while adding the module to the extension.")
                detailedInformation = _("The module has been created, but the extension"
                                        " CMakeLists.txt could not be updated. In order"
                                        " to include the module in the extension build,"
                                        " you will need to update the extension"
                                        " CMakeLists.txt by hand.")
                slicer.util.errorDisplay(text, parent=self.parent.window(), detailedText=traceback.format_exc(),
                                         standardButtons=qt.QMessageBox.Close, informativeText=detailedInformation)

            ExtensionWizardWidget.loadModules(os.path.join(self.extensionLocation, name), depth=0, parent=self.parent.window())
            return

    # ---------------------------------------------------------------------------
    def editExtensionMetadata(self):
        xd = self.extensionDescription
        xp = self.extensionProject

        dlg = EditExtensionMetadataDialog(self.parent.window())
        dlg.project = xp.project
        dlg.description = xd.description
        dlg.contributors = xd.contributors

        if dlg.exec_() == qt.QDialog.Accepted:
            # Update cached metadata
            xd.description = dlg.description
            xd.contributors = dlg.contributors

            # Write changes to extension project file (CMakeLists.txt)
            xp.project = dlg.project
            xp.setValue("EXTENSION_DESCRIPTION", xd.description)
            xp.setValue("EXTENSION_CONTRIBUTORS", xd.contributors)
            xp.save()

            # Update the displayed extension name
            self.extensionNameField.text = xp.project


#
# ExtensionWizardFileDialog
#
class ExtensionWizardFileDialog:
    """This specially named class is detected by the scripted loadable
    module and is the target for optional drag and drop operations.
    See: Base/QTGUI/qSlicerScriptedFileDialog.h
    and commit http://svn.slicer.org/Slicer4/trunk@21951 and issue #3081

    This class offers to load Python scripted modules from a folder
    and append them to the "additional module paths".
    """

    def __init__(self, qSlicerFileDialog):
        self.qSlicerFileDialog = qSlicerFileDialog
        qSlicerFileDialog.fileType = _("Python scripted modules")
        qSlicerFileDialog.description = _("Add Python scripted modules to the application")
        qSlicerFileDialog.action = slicer.qSlicerFileDialog.Read
        self.foundModules = []

    def isMimeDataAccepted(self):
        """Checks the dropped data and returns true if they contain Slicer Python scripted modules"""
        self.foundModules = ExtensionWizardFileDialog.findModulesFromMimeData(self.qSlicerFileDialog.mimeData())
        self.qSlicerFileDialog.acceptMimeData(len(self.foundModules) != 0)

    @staticmethod
    def findModulesFromMimeData(mimeData):
        allFoundModules = []
        if mimeData.hasFormat("text/uri-list"):
            urls = mimeData.urls()
            for url in urls:
                localPath = url.toLocalFile()  # convert QUrl to local path
                allFoundModules.extend(ModuleInfo.findModules(localPath, depth=1))
        return allFoundModules

    def dropEvent(self):
        ExtensionWizardWidget.loadModules(modules=self.foundModules)
        self.foundModules = []
