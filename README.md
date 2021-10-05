# PSD
The pulse shape discrimination

# Discription
This can be used to discriminate between different event types (e.g. electron, proton, ...).
The code in PSD directory is the interface of different PSD method, which define the "IPSDTool". All methods should be **implemented as a tool**, which should inherit "ToolBase" and "IPSDTool"

Two running modes are supported: "Predict" and "preProcess":
* **Predict**: Predict the event type of this event
* **preProcess**: Get the PSD variables of the specfic PSD method, which will be used in the "Training" process

## Input and output
* **Input**: "CalibEvent" and "RecEvent"
* **Output**: 
  * userOutput: reconstruction information, stored in tree "PSDUser"
  * PSD variables: defined by different method, stored in tree "evt"
  * PSD result: The output of PSDEvent, which may be implemented as a data model, stored in tree "PSD"

# To add a new method
See examples in PSD/src/TestPSDTool.*

# Usage
Refer to PSD/share/job.sh
