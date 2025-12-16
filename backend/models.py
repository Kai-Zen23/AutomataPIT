from sqlalchemy import Column, Integer, String, Text, DateTime
from sqlalchemy.sql import func
from .database import Base

class History(Base):
    __tablename__ = "history"

    id = Column(Integer, primary_key=True, index=True)
    mode = Column(Integer) # 1: Regex, 3: Calculator
    input_text = Column(String)
    test_string = Column(String, nullable=True)
    result_output = Column(Text) # Storing the full console output
    created_at = Column(DateTime(timezone=True), server_default=func.now())
