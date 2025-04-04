# Terminologies

## Overview

The Terminologies module provides a coding system for specifying anatomy, clinical findings, or other clinical terms. For example, the coding system can be used to specify what anatomical parts are stored in a segmentation node. Using standard codes reduces the chance of data-entry errors and improves interoperability. The coding system in Slicer is compatible with coding used in DICOM information objects and allows storing SNOMED-CT (Systematized Nomenclature of Human and Veterinary Medicine - Clinical Terms) or any other terminologies.

The module stores multiple lists of terminology items, such a list is called a **context**. Terminologies module comes with two contexts for segmentations (*Segmentation category and type - DICOM master list* and *Segmentation category and type - 3D Slicer General Anatomy list*) and one for specifying region (*Anatomic codes - DICOM master list*). The *3D Slicer General Anatomy list* is a subset of the DICOM master list, containing items corresponding to labels in Slicer's *GenericAnatomyColors* color table. Custom contexts can also be defined. These contexts were created as part of the [QIICR](https://github.com/QIICR) project, in the [dcmqi](https://github.com/QIICR/dcmqi) toolkit.

Each item in the segmentation context specifies **category** (such as "Tissue", "Physical Object", "Body Substance", ...), **type** (such as "Artery", "Bone", "Amygdala", ...), an optional **type modifier** (such as "left" and "right"), recommended **display color**. For items that are not anatomic regions themselves, **region** can be specified as well, using an item from a region context.

Each item in the region context specifies the **region** (such as "Anterior Tibial Artery", "Bladder", ...) and an optional **region modifier** (such as "left" and "right"). Region most commonly specifies an anatomic region, but in non-clinical applications the location may be defined at a much smaller or larger scale, not necessarily at anatomy level.

The terminology module can display user interface (called terminology navigator) for choosing a segmentation terminology item. This navigator appears when selecting the "color" of certain types of data nodes (such as models and markups) in the [Data](data.md) module, or when selecting the "color" for a segment in [Segment Editor](segmenteditor.md). The terminology navigator can also be used to select terminology information from color tables.

## Use cases

Define category, type, type modifier and region of a data object, such as model node or markup node, or a segment in a segmentation node, so that it can be identified unambiguously. The associated standardized codes are also saved into DICOM files when the segmentation is saved as a DICOM Segmentation information object.

## Panels and their use

The terminology navigator dialog is displayed when double-clicking the color selector box in data trees and lists where the color of the objects is shown in a column as a color swatch. Such selectors are there for data nodes in the [Data](data.md), [Models](models.md), [Markups](markups.md) modules, and for the segments within segmentation nodes in the [Segmentations](segmentations.md) and [Segment Editor](segmenteditor.md) modules.

After double-clicking, the basic selector window opens:

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/terminology_selector_basic.png)

It shows the item types in all categories in a searchable list. Custom name and color can be assigned while keeping the selected terminology entry.

Opening up the panes on the left and right, additional options become visible:

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/terminology_selector_advanced.png)

- Left pane
  - Allows selection of a subset of categories (all categories selected by default).
  - On the top, the terminology context can be changed, as well as new ones loaded from `.json` files.
- Right pane
  - Region can be selected for items that can be located in multiple parts of the body, such as "Blood clot", "Mass", or "Cyst".

## How to

- Find items: Start typing the name of the category/type/region in the search box above the column.
- Load new terminology/region context: click the Load button next to the context drop-down and select JSON from local storage.
- Create custom terminology/region context: Start from an existing JSON file, such as the DICOM master list for [terminologies](https://github.com/Slicer/Slicer/blob/main/Modules/Loadable/Terminologies/Resources/SegmentationCategoryTypeModifier-DICOM-Master.json) or [region contexts](https://github.com/Slicer/Slicer/blob/main/Modules/Loadable/Terminologies/Resources/AnatomicRegionAndModifier-DICOM-Master.json). Remove the entries you do not need. Validate the JSON file with the [validator online tool](https://qiicr.org/dcmqi/#/validators).

## References

- [DCMQI wiki](https://github.com/QIICR/dcmqi/wiki)
