def update_log(message, error=False):
    from gui import log_label, log_frame, canvas  # Import only inside function to prevent circular imports

    current_text = log_label.cget("text")
    new_text = current_text + "\n" + message

    log_label.config(text=new_text)

    if error:
        log_label.config(foreground="red")  # Set error messages in red
    else:
        log_label.config(foreground="white")  # Default color for normal messages

    log_label.config(wraplength=400) 

    log_frame.update_idletasks()
    canvas.config(scrollregion=canvas.bbox("all"))
    canvas.yview_moveto(1)