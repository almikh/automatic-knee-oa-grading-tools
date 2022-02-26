import re
import os
import torch
import resnet_family
import densenet_family
import inception_family

def load_model(arch_name, num_classes, filename = None, pretrained=True, device = 'cpu'):
    header = []
    if num_classes == 5: header = ['1', '2', '3', '4', '5']
    elif num_classes == 4: header = ['1', '2', '3', '4']
    else: header = ['1', '2']
    
    if arch_name == "inception_resnet":
        cnn_model = inception_family.inceptionresnetv2_model(num_classes, pretrained)
        frame_size=(299, 299)

    elif "resne" in arch_name:
        frame_size=(224, 224)
        
        if arch_name == "resnet18": cnn_model = resnet_family.resnet18_model(num_classes, pretrained)
        elif arch_name == "resnet34": cnn_model = resnet_family.resnet34_model(num_classes, pretrained)
        elif arch_name == "resnet50": cnn_model = resnet_family.resnet50_model(num_classes, pretrained)
        elif arch_name == "se_resnet18": cnn_model = resnet_family.se_resnet18_model(num_classes, pretrained)
        elif arch_name == "se_resnet34": cnn_model = resnet_family.se_resnet34_model(num_classes, pretrained)
        elif arch_name == "se_resnet50": cnn_model = resnet_family.se_resnet50_model(num_classes, pretrained)
        elif arch_name == "resnext": cnn_model = resnet_family.resnext50_model(num_classes, pretrained)
        
    elif "dense" in arch_name:
        frame_size=(224, 224)
    
        if arch_name == "densenet121": cnn_model = densenet_family.densenet121_model(num_classes, pretrained)
        elif arch_name == "se_densenet121": cnn_model = densenet_family.se_densenet121_model(num_classes, pretrained)
            
    elif "ception" in arch_name:    
        frame_size=(299, 299)
    
        if arch_name == "inception_v3": cnn_model = inception_family.inception_v3(num_classes, pretrained)
        if arch_name == "se_inception_v3": cnn_model = inception_family.se_inception_v3(num_classes, pretrained)
        elif arch_name == "xception": cnn_model = inception_family.xception_model(num_classes, pretrained)
        elif arch_name == "se_xception": cnn_model = inception_family.se_xception_model(num_classes, pretrained)

    if filename is not None:
        state_dict = torch.load(filename, map_location=torch.device(device))
        cnn_model.load_state_dict(state_dict)
            
    return cnn_model, frame_size, header

def compute_accuracy(model, loader):
    model.eval() # Evaluation mode
    
    predictions = []
    raw_predictions = []
    ground_truth = []

    total_samples = 0
    correct_samples = 0
    for i_step, (x, y) in enumerate(loader):
        x_gpu = x
        y_gpu = y
        
        prediction = model(x_gpu) 

        indices = torch.argmax(prediction, 1)
        correct_samples += torch.sum(indices == y_gpu)
        total_samples += y.shape[0]
            
        # store result
        raw_predictions.extend(prediction.tolist())
        predictions.extend(indices.tolist())
        ground_truth.extend(y.tolist())

    return float(correct_samples) / total_samples, predictions, ground_truth, raw_predictions
