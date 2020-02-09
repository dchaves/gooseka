FROM python:3.8.1-alpine3.11
RUN mkdir -p /controller
RUN pip install --upgrade pip && pip install inputs pyserial
ADD gooseka_controller.py /controller
CMD ["python3","/controller/gooseka_controller.py"]