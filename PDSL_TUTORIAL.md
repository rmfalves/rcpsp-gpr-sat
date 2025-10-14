### **1. Introduction**
This document describes the syntax and format of a project definition script to pass as parameter to the solver.

### **2. Comments**
Commentary lines start by #. Blank lines are also accepted. 

### **3. Header**
PROJECT *num_activities* *makespan* *num_resources*.

This must be the first line. Set the number of project activities as *num_activities*, the project makespan as *makespan*, and the number of project resources as *num_resources*.

### **4. Tasks duration**
ACTIVITY *activity_number*, *t*

Set the activity given by *activity_number* with a duration of *t* time units, where *activity_number* ranges from 1 to the number of activities and *t* is a positive integer not greater than the project makespan.

### **5. Resources capacities**
RESOURCE *resource_number*, *capacity*

Set the resource given by *resource_number* with a capacity given by *capcity*, where *resource_number* ranges from 1 to the number of resources and *capacity* is a positive integer.

### **6. Activity dependencies**
SEQUENCE *activity_x*, *activity_y*, *sequence_type*

Set a dependency from *activity_x* to *activity_y*, where *activity_x* and *activity_y* range from 1 to the number of activities and *sequence_type* is one of the four types *FS*, *FF*, *SS*, and *SF*.

### **7. Resources allocation**
ALLOCATE *activity_number*, *resource_number*, *x*

Allocate *x* units of the resource given by *resource_number* to the activity given by *activity_number*, where *activity_number* ranges from 1 to the number of activities, *resource_number* ranges from 1 to the number of resources and *x* is a positive integer not greater than the capacity of the given resource.