nn_yolov8 examples based on MaixCDK
====
- Use `maixcdk build` to compile binary files.
- Move files and runtime libraries to device.
- Use SSH command with parameters to run programs.
  
` "Usage: " + std::string(argv[0]) + " mud_model_path <image_path>"; `

To use "yolov8n_pose" you can add [draw_pose](https://wiki.sipeed.com/maixpy/api/maix/nn.html#draw_pose)

To use "yolov8n_seg" you can add [draw_seg_mask](https://wiki.sipeed.com/maixpy/api/maix/nn.html#draw_seg_mask)
