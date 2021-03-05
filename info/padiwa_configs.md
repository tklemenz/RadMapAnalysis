# Some general info

## Fiber numbering

                     board I
          _____________________________. . .__
         /32                                32/
         /                                    /
         /                                    /
         /                                    /
         /                                    /
         /                                    /
board IV /                                    /     board III
         .                                    .
         .                                    .
         .3                                   .
         /2                                   /
         /1 2 3 4_______________. . ._________/

                     board II



## General remarks: 
   * layers count from 1 to 8
   * odd layers are in y, even layers are in x direction
   * top layer always vertical!!
   * board I always reverted even
   * board II always odd
   * board III always odd
   * board IV always reverted even


## Padiwa configs

### 0: read out first 4 layers (2-2-2-2)   [layer 5-8: just do Adapterboard slot +2 for all Padiwas]

| Padiwa | TDC | Adapterboard | Slot | TDC channels | Fiber mapping | Layer | Direction |
| ------ |:---:| ------------ |:----:| ------------ | ------------- |:-----:|:---------:|
| 1500_0 | 0   | I            | 1    | 1-16         | reverted even | 1     | Y         |
| 1500_1 | 0   | II           | 1    | 17-32        | odd           | 1     | Y         |
| 1510_0 | 1   | III          | 1    | 1-16         | odd           | 2     | X         |
| 1510_1 | 1   | IV           | 1    | 17-32        | reverted even | 2     | X         |
| 1520_0 | 2   | I            | 2    | 1-16         | reverted even | 3     | Y         |
| 1520_1 | 2   | II           | 2    | 17-32        | odd           | 3     | Y         |
| 1530_0 | 3   | III          | 2    | 1-16         | odd           | 4     | X         |
| 1530_1 | 3   | IV           | 2    | 17-32        | reverted even | 4     | X         |


### 1: three full layers in x direction + two aligned half layers in y direction 

| Padiwa | TDC | Adapterboard | Slot | TDC channels | Fiber mapping | Layer | Direction |
| ------ |:---:| ------------ |:----:| ------------ | ------------- |:-----:|:---------:|
| 1500_0 | 0   | I            | 1    | 1-16         | reverted even | 1     | Y         |
| 1500_1 | 0   | II           | 1    | 17-32        | odd           | 1     | Y         |
| 1510_0 | 1   | I            | 2    | 1-16         | reverted even | 3     | Y         |
| 1510_1 | 1   | II           | 2    | 17-32        | odd           | 3     | Y         |
| 1520_0 | 2   | I            | 3    | 1-16         | reverted even | 5     | Y         |
| 1520_1 | 2   | II           | 3    | 17-32        | odd           | 5     | Y         |
| 1530_0 | 3   | IV           | 1    | 1-16         | reverted even | 2     | X         |
| 1530_1 | 3   | IV           | 3    | 17-32        | reverted even | 6     | X         |


### 2: three full layers in y direction + one full layer in x direction

| Padiwa | TDC | Adapterboard | Slot | TDC channels | Fiber mapping | Layer | Direction |
| ------ |:---:| ------------ |:----:| ------------ | ------------- |:-----:|:---------:|
| 1500_0 | 0   | I            | 1    | 1-16         | reverted even | 1     | Y         |
| 1500_1 | 0   | II           | 1    | 17-32        | odd           | 1     | Y         |
| 1510_0 | 1   | I            | 2    | 1-16         | reverted even | 3     | Y         |
| 1510_1 | 1   | II           | 2    | 17-32        | odd           | 3     | Y         |
| 1520_0 | 2   | I            | 3    | 1-16         | reverted even | 5     | Y         |
| 1520_1 | 2   | II           | 3    | 17-32        | odd           | 5     | Y         |
| 1530_0 | 3   | III          | 1    | 1-16         | odd           | 2     | X         |
| 1530_1 | 3   | IV           | 1    | 17-32        | reverted even | 2     | X         |


### 3: like 0 but Padiwas within Adapterboard switched

| Padiwa | TDC | Adapterboard | Slot | TDC channels | Fiber mapping | Layer | Direction |
| ------ |:---:| ------------ |:----:| ------------ | ------------- |:-----:|:---------:|
| 1500_0 | 0   | I            | 2    | 1-16         | reverted even | 3     | Y         |
| 1500_1 | 0   | II           | 2    | 17-32        | odd           | 3     | Y         |
| 1510_0 | 1   | III          | 2    | 1-16         | odd           | 4     | X         |
| 1510_1 | 1   | IV           | 2    | 17-32        | reverted even | 4     | X         |
| 1520_0 | 2   | I            | 1    | 1-16         | reverted even | 1     | Y         |
| 1520_1 | 2   | II           | 1    | 17-32        | odd           | 1     | Y         |
| 1530_0 | 3   | III          | 1    | 1-16         | odd           | 2     | X         |
| 1530_1 | 3   | IV           | 1    | 17-32        | reverted even | 2     | X         |