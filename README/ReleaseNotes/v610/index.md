% ROOT Version 6.10 Release Notes
% 2016-09-30
<a name="TopOfPage"></a>

## Introduction

ROOT version 6.10/00 is scheduled for release in 2017.

For more information, see:

[http://root.cern.ch](http://root.cern.ch)

The following people have contributed to this new version:

 Bertrand Bellenot, CERN/SFT,\
 Georgios Bitzes, CERN/IT,\
 Rene Brun, CERN/SFT,\
 Philippe Canal, FNAL,\
 Olivier Couet, CERN/SFT,\
 Gerri Ganis, CERN/SFT,\
 Andrei Gheata, CERN/SFT,\
 Sergey Linev, GSI, http,\
 Pere Mato, CERN/SFT,\
 Lorenzo Moneta, CERN/SFT,\
 Axel Naumann, CERN/SFT,\
 Danilo Piparo, CERN/SFT,\
 Fons Rademakers, CERN/SFT,\
 Enric Tejedor Saavedra, CERN/SFT,\
 Vassil Vassilev, Fermilab/CMS,\
 Wouter Verkerke, NIKHEF/Atlas, RooFit

## Removed interfaces

The following interfaces have been removed, after deprecation in v6.08.

### CINT remnants, dysfunctional for ROOT 6

- `TInterpreter`'s `Getgvp()`, `Getp2f2funcname(void*)`, `Setgvp(Long_t)`, `SetRTLD_NOW()`, `SetRTLD_LAZY()`.
- `SetFCN(void*)` from TVirtualFitter, TFitter, TBackCompFitter, TMinuit
- `TFoam::SetRhoInt(void*)`

## Interpreter

- Automatic declaration of variables (`h = new TH1F(...)`) is *only* available at the prompt. The side-effects of relying on this in source files is simply too grave. Due to a bug (ROOT-8538), automatically declared variables must currently reside on the top-most scope, i.e. not inside an `if` block etc.
- Improved the stack frame information generated by the JIT.  By avoiding interleaving of the memory associated to multiple JIT module, the generation of stack trace involving jitted code and the catching of exception going through jitted code has been repaired.

## Core Libraries

- See "Build, Configuration and Testing Infrastructure" below for changes in the directory structure.
- libCling now exports only a minimal set of symbols.
- Add support for std::array_view also for C++11 builds. The implementation has been modified to work before C++14.

## Histogram Libraries

- New class `THnChain` was added to provide a `TChain`-like experience when
  working with `THnBase`'ed histograms (currently `THn` and `THnSparse`) from
  many files, see [here](https://sft.its.cern.ch/jira/browse/ROOT-4515). This
  allows to e.g., interactively adjust axis parameters before performing
  projections from high-dimensional histograms,

  ```{.cpp}
  // Create a chain of histograms called `h`.
  THnChain chain("h");

  // Add files containing histograms `h` to `chain`.
  chain->AddFile("file1.root");

  chain->GetXaxis(1)->SetRangeUser(0.1, 0.2);

  TH1* projection = chain->Projection(0)
  ```


## Math Libraries


## RooFit Libraries


## TTree Libraries

- `TTreeReader` now supports `TEntryList`s.
- `TTreeReader::SetLastEntry()` has been deprecated. Its name is misleading; please use `TTreePlayer::SetEntriesRange()` instead.
- `TTree::Branch()` now complains for wrong leaf list strings, e.g. "value/F[4]" (which should really be spelled as "value[4]/F").
- Allow reading of older version of TTreePerfStats (ROOT-8520)
- Introduce TDataFrame which offers a new and highly efficient way to analyse data stored in TTrees

## 2D Graphics Libraries
- If one used "col2" or "colz2", the value of `TH1::fMaximum` got modified.
  This deviated from the behavior of "col" or "colz". This is now fixed as
  requested [here](https://sft.its.cern.ch/jira/browse/ROOT-8389).
- When the option SAME (or "SAMES") is used with the option COL, the boxes' color
  are computing taking the previous plots into account. The range along the Z axis
  is imposed by the first plot (the one without option SAME); therefore the order
  in which the plots are done is relevant.
- With option BOX on 2D histos with negative content:
    - do not draw the empty bins as requested [here](https://sft.its.cern.ch/jira/browse/ROOT-8385).
    - fix the issue mentioned [here](https://sft.its.cern.ch/jira/browse/ROOT-*402).
- When several histogram were drawn on top of each other with the option
  `BOX SAME` and if the log scale along Z was on, the plot showed only the
  first histogram. This can be reproduce by using the documentation example
  illustrating `BOX SAME`and turning the canvas into log scale along Z.
- In TLatex:
    - Do not paint the text when the text size is <= 0. This fixes
      the problem mentioned [here](https://sft.its.cern.ch/jira/browse/ROOT-8305)
    - Do not paint text if the text string is empty.
- From: Sergey Linev: In `TPad::SaveAs` method json file extension is now handled
- Because of some precision issue some data points exactly on the plot limits of
  a `TGraph2D` were not drawn (option `P`).
  The problem was reported [here](https://sft.its.cern.ch/jira/browse/ROOT-8447).
- New options for automatic coloring of graphs and histograms. When several
  histograms or graphs are painted in the same canvas thanks to the option "SAME"
  via a `THStack` or `TMultigraph` it might be useful to have an easy and automatic
  way to choose their color. The simplest way is to pick colors in the current active color
  palette. Palette coloring for histogram is activated thanks to the options `PFC`
  (Palette Fill Color), `PLC` (Palette Line Color) and `AMC` (Palette Marker Color).
  When one of these options is given to `TH1::Draw` the histogram get its color
  from the current color palette defined by `gStyle->SetPalette(…)`. The color
  is determined according to the number of objects having palette coloring in
  the current pad.
- The line width and line style can be change on 2d histograms painted with
  option `ARR`.
- When the angle of a TGraphPolar was not in radian, the error bars were misplaced.
  The problem was reported [here](https://sft.its.cern.ch/jira/browse/ROOT-8476).
- In `TASimage::DrawLineInternal` the case of a line with 0 pixel along X and 0
  pixel along Y was not treated properly. An horizontal line was drawn instead.
- In `TGraphPainter::PaintGrapHist`: Decouple the `P` option (histogram drawn with
  a simple polymarker) from the `L`(Histogram drawn as a simple polyline). This
  improved (in some cases some extra markers were drawn) and simplify. the code.
- Candle plot improvements:
   * Rearragement of TCandle-code - split into calculate and paint
   * Implementation for a "raw-data candle" inside TCandle - to be used from TTreeViewer in the future
   * Implementation of 1D histograms along each candle (left, right and violin) - to be used for violin-charts
   * Implementation of a zero indicator line for TCandle - to be used for violin-charts
   * Reimplementation if THistPainter draw option VIOLIN
   * Implementations of presets and individual options for VIOLIN-charts
   * Implementation of VIOLIN-charts in THStack - can be combined with CANDLE
   * Update of the docs (THistPainter and THStack)
   * New tutorials
- In various places in TGraph the underlying histogram was deleted when the graph
  range should be recomputed. This has the side effect that some graph parameters
  (like the axis titles) were also deleted. This now fixed. It was reported
  [here](https://sft.its.cern.ch/jira/browse/ROOT-8092).
- Improve the error bars drawing in TLegend to match the plot's error
  drawing. This improvement was requested [here](https://sft.its.cern.ch/jira/browse/ROOT-5468).
- Implement text clipping in TASImage as requested [here](https://sft.its.cern.ch/jira/browse/ROOT-4538).
  Also the text size in batch mode for png (gif jpeg) files better matches the
  size on screen and pdf.
- `TMathText` and `TTeXDump` implement the `TLatex` character `\bar`.
- In the following example, `TPad::WaitPrimitive` was not stoping the macro
  execution after each plot :
~~~ {.cpp}
{
   TCanvas c1("c1");
   TFile f("hsimple.root");
   hpx->Draw();        gPad->WaitPrimitive();
   hpxpy->Draw();      gPad->WaitPrimitive();
   hprof->Draw();
~~~
  this was reported [here](https://root.cern.ch/phpBB3/viewtopic.php?f=3&t=22957).
- New flag `Cocoa.EnableFillAreaAntiAliasing` in `system.rootrc` to enable the
  anti-aliasing for filled area for the Cocoa backend. Default is `no`.
- The "BOX" option, to draw 3D histograms, has been reimplemented by Evgueni Tcherniaev
  The following picture show the old and new version

![New box option for 3D histograms](NewBoxOption.png)

- Implement options "BOX1" and "BOX3" for TH3 equivalent of "LEGO1" and "LEGO3"for TH2.
- When a 2d histogram was drawn with option `LEGO1` and white colored, the dark side
  of the lego was red instead of gray.
- New option "0" to draw TH2Poly. When used with any `COL` options, the empty
  bins are not drawn.
- Fix a long pending problem with Z axis drawing when a lego or a surface was drawn
  upside-down.

## 3D Graphics Libraries
- In `TMarker3DBox::PaintH3` the boxes' sizes was not correct.
- The option `BOX`and `GLBOX` now draw boxes with a volume proportional to the
  bin content to be conform to the 2D case where the surface of the boxes is
  proportional to the bin content.

## Geometry Libraries

## Dictionaries
- Stop dictionary generation early, during AST scanning, if a union is selected for I/O as this is not supported (triggered by [ROOT-8492](https://sft.its.cern.ch/jira/browse/ROOT-8492))
- Allow inclusion of headers in linkdef files [ROOT-7765](https://sft.its.cern.ch/jira/browse/ROOT-7765)
- More expressive error messages when trying to directly select std::arrays

## I/O Libraries
- [[ROOT-8478](https://sft.its.cern.ch/jira/browse/ROOT-8478)] - Prompt error when building streamer info and a data member is a vector<T> w/o dictionary
- TDavixFile: Added support for bucket name in path
- Fix error sometimes prompted when trying to write std::array column-wise

## Database Libraries


## Networking Libraries


## GUI Libraries


## Montecarlo Libraries


## Parallelism and PROOF
- Add ROOT::GetImplicitMTPoolSize function to get the size of the pool used to enable implicit multi threading

## Language Bindings


## JavaScript ROOT


## Tutorials


## Class Reference Guide


## Build, Configuration and Testing Infrastructure

- rlibmap has been removed; it was deprecated for three years.
- Added the CMake exported ROOT libraries into the ROOT:: namespace. In this way, projects based on CMake using ROOT can avoid
  conflicts in library target names. As an example, this is the way to build a project consisting of one library and one
  executable using ROOT.
  ```
  find_package(ROOT REQUIRED)
  include(${ROOT_USE_FILE})

  include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
  add_definitions(${ROOT_CXX_FLAGS})

  ROOT_GENERATE_DICTIONARY(G__Event Event.h LINKDEF EventLinkDef.h)

  add_library(Event SHARED Event.cxx G__Event.cxx)
  target_link_libraries(Event ROOT::Hist ROOT::Tree)

  add_executable(Main MainEvent.cxx)
  target_link_libraries(Main Event)
  ```
- Added option `builtin_all` to enable all the built in options.
- For rootcling_stage1 (formerly known as rootcling_tmp), the package structure was changed to enable homogenous visibility
  settings across object files. See core/README for an overview.
- Several non-public headers are not copied into include/ anymore; they reside in the PACKAGE/res/ subdirectory in the source tree.
