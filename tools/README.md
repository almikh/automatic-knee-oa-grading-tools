# Tools

List of available scripts:

- `train.ipynb`: script for training the selected convolutional network model for a given set of random seeds (21, 42, 84). See section `Settings`.

- `train_single.ipynb`: script for training the selected convolutional network model for a fixed random seed. See section `Settings`.

- `test_single.ipynb`: script for testing the selected convolutional network model for the specified checkpoint. Displays all information on the test results. See section `Settings`.

- `test_ensemble.ipynb`: script for testing the ensemble of selected convolutional network architecture for the specified checkpoints. Displays all information on the test results. See section `Settings`.

- `print_models_params.ipynb`: displays the number of network parameters used architectures.

- `metrics_finder.ipynb`: auxiliary scripts for displaying statistics and collecting information on several checkpoints of models of the same type.

- `converter.ipynb`: A utility for converting a model from a format obtained during training into a format suitable for use in native code.
